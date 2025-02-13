#include <gtest/gtest.h>
#include <vector>
#include <sstream>
#include <filesystem>
#include <fstream>
#include "cmdparser.h"
#include "walker.h"
#include "comparer.h"
#include "hashes.h"
using namespace std;
namespace fs = std::filesystem;

Settings makeDefaultSettings(const std::string& args)
{
	stringstream ss(args);
	string token;
	vector<string> vArgs;
	while (std::getline(ss, token, ' '))
		vArgs.push_back(token);

	std::vector<char*> argv;
	for (std::string& s : vArgs)
		argv.push_back(&s[0]);
	argv.push_back(nullptr);

	CmdParser parser;
	auto [settings, helpString] = parser.parse(argv.size() - 1, argv.data());
	return settings;
}
Settings makeToplevelSettings()
{
	return makeDefaultSettings("bayan -I a/b a/c notexists -E excluded -M .*\\.md .*\\.txt -D 0");
}
Settings makeRecursiveSettings()
{
	return makeDefaultSettings("bayan -I a/b a/c notexists -E excluded -M .*\\.md .*\\.txt -D 1");
}


TEST(Bayan, DefaultCommandLine)
{
	string args = "bayan -B 10";
	stringstream ss(args);
	string token;
	vector<string> vArgs;
	while (std::getline(ss, token, ' '))
		vArgs.push_back(token);

	std::vector<char*> argv;
	for (std::string& s : vArgs)
		argv.push_back(&s[0]);
	argv.push_back(nullptr);


	CmdParser parser;
	auto [settings, helpString] = parser.parse(argv.size() - 1, argv.data());
	EXPECT_FALSE(settings.justPrintHelp);
	EXPECT_EQ(".", settings.includeDirectories[0]);
	EXPECT_TRUE(settings.excludeDirectories.empty());
	EXPECT_EQ(".*", settings.enabledFileMasks[0]);
	EXPECT_EQ(Settings::ScanDepth::RECURSIVE, settings.scanDepth);
	EXPECT_EQ(1, settings.minFileSize);
	EXPECT_EQ(10, settings.readBlockSize); // NOTE not checking for block size in this test
	EXPECT_EQ(Settings::HashType::CRC32, settings.hashType);
}

TEST(Bayan, WrongCommandLine)
{
	string args = "bayan -I inc1 inc2 -TT 1"; // NOTE last option leads to exception
	stringstream ss(args);
	string token;
	vector<string> vArgs;
	while (std::getline(ss, token, ' '))
		vArgs.push_back(token);

	std::vector<char*> argv;
	for (std::string& s : vArgs)
		argv.push_back(&s[0]);
	argv.push_back(nullptr);

	CmdParser parser;
	Settings settings;
	string helpString;
	EXPECT_NO_THROW(std::tie(settings, helpString) = parser.parse(argv.size() - 1, argv.data()));
	EXPECT_TRUE(settings.justPrintHelp);
}

TEST(Bayan, CommandLine)
{
	string args = "bayan -I inc1 inc2 -E ex1 ex2 -M .*\\.md .*\\.txt -D 0 -S 2 -B 20 -H 1";
	stringstream ss(args);
	string token;
	vector<string> vArgs;
	while (std::getline(ss, token, ' '))
		vArgs.push_back(token);

	std::vector<char*> argv;
	for (std::string& s : vArgs)
		argv.push_back(&s[0]);
	argv.push_back(nullptr);

	CmdParser parser;
	auto [settings, helpString] = parser.parse(argv.size()-1, argv.data());
	EXPECT_FALSE(settings.justPrintHelp);
	EXPECT_EQ("inc1", settings.includeDirectories[0]);
	EXPECT_EQ("inc2", settings.includeDirectories[1]);
	EXPECT_EQ("ex1", settings.excludeDirectories[0]);
	EXPECT_EQ("ex2", settings.excludeDirectories[1]);
	EXPECT_EQ(".*\\.md", settings.enabledFileMasks[0]);
	EXPECT_EQ(".*\\.txt", settings.enabledFileMasks[1]);
	EXPECT_EQ(Settings::ScanDepth::TOPLEVEL, settings.scanDepth);
	EXPECT_EQ(2, settings.minFileSize);
	EXPECT_EQ(20, settings.readBlockSize);
	EXPECT_EQ(Settings::HashType::MD5, settings.hashType);
}

void createFile(const string& path, const string& content)
{
	fs::create_directories(fs::path(path).parent_path());
	ofstream f(path, ios::out | ios::trunc);
	f << content;
	f.close();
}

class BayanDirs : public ::testing::Test 
{
public:
	static void SetUpTestSuite()
	{
		createFile("a/b/first.txt", "Hello, world!");
		createFile("a/b/found_only_by_walker.txt", "different text 1");
		createFile("a/c/first_.TXT", "Hello, world!");
		createFile("a/c/first_copy.txt", "Hello, world!");
		createFile("a/c/found_only_by_walker.md", "different text 2");
		createFile("a/c/excluded/first.md", "Hello, world!");
		createFile("a/c/excluded/should_not_be_found.txt", "aaaaaaaaaaaaaaaa");
		createFile("a/c/excluded/first.md", "Hello, world!");
		createFile("a/c/d/first.md", "Hello, world!");
		createFile("a/c/d/filtered.ext", "aaaaaaaaaaaaaaaa");
		createFile("a/c/d/second.md", "some text");
		createFile("a/c/e/filtered.ext", "aaaaaaaaaaaaaaaa");
		createFile("a/c/e/others_are_filtered.txt", "aaaaaaaaaaaaaaaa"); 
		createFile("a/c/e/first.md", "Hello, world!");
		createFile("a/c/e/first.txt", "Hello, world!");
		createFile("a/c/e/second.txt", "some text");
		createFile("a/c/e/found_only_by_walker.md", "different text 3");
	}
	static void TearDownTestSuite()
	{
	}
};

TEST_F(BayanDirs, WalkDirectoriesTopLevel)
{
	struct WalkerDirectories : public Walker
	{ // adds just directories to res[1]
		WalkerDirectories(const Settings& s) : Walker(s) {}
		virtual void parseFilesInDirectory(const std::string& directory, results_type& res)
		{
			if (res.count(1) == 0)
				res[1] = std::set<string>();
			res[1].insert(directory);
		}
	};

	Settings s = makeToplevelSettings();
	WalkerDirectories w(s);
	Walker::results_type res = w.run();
	EXPECT_EQ(1, res.count(1)); // res[1] should contain all directories 
 	const std::set<std::string>& dirs = res.at(1);
 	EXPECT_EQ(2, dirs.size());

	namespace fs = std::filesystem; // NOTE converting slashes
	EXPECT_EQ(1, dirs.count(fs::path{ "a/b" }.make_preferred().string()));
	EXPECT_EQ(1, dirs.count(fs::path{ "a/c" }.make_preferred().string()));
}

TEST_F(BayanDirs, WalkDirectoriesRecursive)
{
	struct WalkerDirectories : public Walker
	{ // adds just directories to res[1]
		WalkerDirectories(const Settings& s) : Walker(s) {}
		virtual void parseFilesInDirectory(const std::string& directory, results_type& res)
		{
			if (res.count(1) == 0)
				res[1] = std::set<string>();
			res[1].insert(directory);
		}
	};

	Settings s = makeRecursiveSettings();
	WalkerDirectories w(s);
	Walker::results_type res = w.run();
	EXPECT_EQ(1, res.count(1)); // res[1] should contain all directories 
	const std::set<std::string>& dirs = res.at(1);
	EXPECT_EQ(4, dirs.size());

	namespace fs = std::filesystem; // NOTE converting slashes
	EXPECT_EQ(1, dirs.count(fs::path{ "a/b" }.make_preferred().string()));
	EXPECT_EQ(1, dirs.count(fs::path{ "a/c" }.make_preferred().string()));
	EXPECT_EQ(1, dirs.count(fs::path{ "a/c/d" }.make_preferred().string()));
	EXPECT_EQ(1, dirs.count(fs::path{ "a/c/e" }.make_preferred().string()));
}

TEST_F(BayanDirs, WalkFilesTopLevel)
{
	Settings s = makeToplevelSettings();
	Walker w(s);
	Walker::results_type res = w.run();
	EXPECT_EQ(2, res.size());
	EXPECT_EQ(3, res.at(13).size()); // 3 files with size=13 bytes
	EXPECT_EQ(2, res.at(16).size()); // 2 files with size=16 bytes
}

TEST_F(BayanDirs, WalkFilesRecursive_Crc32)
{
	Settings s = makeRecursiveSettings();
	Walker w(s);
	Walker::results_type res = w.run();
	EXPECT_EQ(3, res.size());
	EXPECT_EQ(2, res.at(9).size()); // 2 files with size=9 bytes
	EXPECT_EQ(6, res.at(13).size()); // 6 files with size=13 bytes
	EXPECT_EQ(4, res.at(16).size()); // 3 files with size=16 bytes
}

TEST_F(BayanDirs, WalkFilesRecursive_Md5)
{
	Settings s = makeRecursiveSettings();
	s.hashType = Settings::HashType::MD5;
	Walker w(s);
	Walker::results_type res = w.run();
	EXPECT_EQ(3, res.size());
	EXPECT_EQ(2, res.at(9).size()); // 2 files with size=9 bytes
	EXPECT_EQ(6, res.at(13).size()); // 6 files with size=13 bytes
	EXPECT_EQ(4, res.at(16).size()); // 3 files with size=16 bytes
}

TEST_F(BayanDirs, SimpleCompareIdentical)
{
	Settings settings = makeRecursiveSettings();
	Walker::results_type walkerRes;
	set<string> files{
		fs::path{ "a/b/first.txt" }.make_preferred().string(),
		fs::path{ "a/c/first_copy.txt" }.make_preferred().string()
	};

	walkerRes[13] = files;

	Comparer comp(settings);
	vector<vector<string>> res = comp.compare(13, files);
	EXPECT_EQ(1, res.size());
	EXPECT_EQ(2, res[0].size());
}

class BayanGroups : public ::testing::Test
{
public:
	static void SetUpTestSuite()
	{
		createFile("a/id/different/1.txt", "11");
		createFile("a/id/different/2.txt", "22");
		createFile("a/id/different/3.txt", "33");
		createFile("a/id/two_eq_groups/a.txt", "first content");
		createFile("a/id/two_eq_groups/a_copy.txt", "first content");
		createFile("a/id/two_eq_groups/b.txt", "second content");
		createFile("a/id/two_eq_groups/b_copy.txt", "second content");
		createFile("a/id/two_three/a.txt", "first content");
		createFile("a/id/two_three/a_copy.txt", "first content");
		createFile("a/id/two_three/b.txt", "second content");
		createFile("a/id/two_three/b_copy.txt", "second content");
		createFile("a/id/two_three/b_another_copy.txt", "second content");
	}
	static void TearDownTestSuite()
	{
	}
};

TEST_F(BayanGroups, EqualTwoGroups_Crc32)
{
	Settings settings = makeRecursiveSettings();
	Walker::results_type walkerRes;
	set<string> files{ 
		fs::path{ "a/id/two_eq_groups/a.txt" }.make_preferred().string(),
		fs::path{ "a/id/two_eq_groups/a_copy.txt" }.make_preferred().string(),
		fs::path{ "a/id/two_eq_groups/b.txt" }.make_preferred().string(),
		fs::path{ "a/id/two_eq_groups/b_copy.txt" }.make_preferred().string()
	};
	walkerRes[13] = files;

	Comparer comp(settings);
	vector<vector<string>> res = comp.compare(13, files);
	EXPECT_EQ(2, res.size()); // 2 groups of files
	EXPECT_EQ(2, res[0].size()); // first group = 2 files
	EXPECT_EQ(2, res[1].size()); // second group = 2 files
}

TEST_F(BayanGroups, EqualTwoGroups_Md5)
{
	Settings settings = makeRecursiveSettings();
	settings.hashType = Settings::HashType::MD5;
	Walker::results_type walkerRes;
	set<string> files{
		fs::path{ "a/id/two_eq_groups/a.txt" }.make_preferred().string(),
		fs::path{ "a/id/two_eq_groups/a_copy.txt" }.make_preferred().string(),
		fs::path{ "a/id/two_eq_groups/b.txt" }.make_preferred().string(),
		fs::path{ "a/id/two_eq_groups/b_copy.txt" }.make_preferred().string()
	};
	walkerRes[13] = files;

	Comparer comp(settings);
	vector<vector<string>> res = comp.compare(13, files);
	EXPECT_EQ(2, res.size()); // 2 groups of files
	EXPECT_EQ(2, res[0].size()); // first group = 2 files
	EXPECT_EQ(2, res[1].size()); // second group = 2 files
}

TEST_F(BayanGroups, EqualTwoThreeGroups)
{
	Settings settings = makeRecursiveSettings();
	Walker::results_type walkerRes;
	set<string> files{
		fs::path{ "a/id/two_three/a.txt" }.make_preferred().string(),
		fs::path{ "a/id/two_three/a_copy.txt" }.make_preferred().string(),
		fs::path{ "a/id/two_three/b.txt" }.make_preferred().string(),
		fs::path{ "a/id/two_three/b_copy.txt" }.make_preferred().string(),
		fs::path{ "a/id/two_three/b_another_copy.txt" }.make_preferred().string()
	};
	walkerRes[13] = files;

	Comparer comp(settings);
	vector<vector<string>> res = comp.compare(13, files);
	EXPECT_EQ(2, res.size()); // 2 groups of files
	size_t sz1 = res[0].size();
	size_t sz2 = res[1].size();
	if (sz1 > sz2)
		std::swap(sz1, sz2);
	EXPECT_EQ(2, sz1); // lesser group consists of 2 files
	EXPECT_EQ(3, sz2); // greater group consists of 3 files
}

TEST_F(BayanGroups, Different_Crc32)
{
	Settings settings = makeRecursiveSettings();
	Walker::results_type walkerRes;
	set<string> files{
		fs::path{ "a/id/different/1.txt" }.make_preferred().string(),
		fs::path{ "a/id/different/2.txt" }.make_preferred().string(),
		fs::path{ "a/id/different/3.txt" }.make_preferred().string()
	};
	walkerRes[13] = files;

	Comparer comp(settings);
	vector<vector<string>> res = comp.compare(2, files);
	EXPECT_EQ(0, res.size()); 
}

TEST_F(BayanGroups, Different_Md5)
{
	Settings settings = makeRecursiveSettings();
	settings.hashType = Settings::HashType::MD5;
	Walker::results_type walkerRes;
	set<string> files{
		fs::path{ "a/id/different/1.txt" }.make_preferred().string(),
		fs::path{ "a/id/different/2.txt" }.make_preferred().string(),
		fs::path{ "a/id/different/3.txt" }.make_preferred().string()
	};
	walkerRes[13] = files;

	Comparer comp(settings);
	vector<vector<string>> res = comp.compare(2, files);
	EXPECT_EQ(0, res.size());
}

