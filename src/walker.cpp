#include "walker.h"
#include <filesystem> 
#include <algorithm>
using namespace std;
namespace fs = std::filesystem;

Walker::Walker(const Settings& settings) : m_settings(settings) {
	// set up regexes
	for (const string& s : m_settings.enabledFileMasks)
		m_fileRegexes.emplace_back( regex{ s, regex_constants::icase } );
}

Walker::results_type Walker::run()
{
	Walker::results_type results;

	for (const std::string& inclDir : m_settings.includeDirectories)
	{
		if (!fs::exists(inclDir) || isDirectoryExcluded(inclDir))
			continue;

		parseFilesInDirectory(inclDir, results);

		if (m_settings.scanDepth == Settings::ScanDepth::TOPLEVEL)
			continue;

		for (const fs::directory_entry& innerDir : fs::recursive_directory_iterator{ inclDir })
		{
			const std::string& innerDirStr = innerDir.path().string();
			if (innerDir.is_directory() && ! isDirectoryExcluded(innerDirStr) )
				parseFilesInDirectory(innerDirStr, results);
		}
	}

	// erase all keys with just one file
	// NOTE can use erase_if in c++20
	for (auto it = results.begin(); it != results.end(); )
	{
		if (it->second.size() <= 1)
			results.erase(it++);
		else
			++it;
	}

	return results;
}

bool Walker::isDirectoryExcluded(const std::string& dir) const
{
	const vector<string>& excl = m_settings.excludeDirectories;
 	return find_if(excl.begin(), excl.end(), [&](const string& ex) { 
 		return dir.find(ex) != string::npos; // path not contains directory from commandline excludes
	}) != excl.end();
	return false;
}

void Walker::parseFilesInDirectory(const string& directory, Walker::results_type &res)
{
	for (const fs::directory_entry& f : fs::directory_iterator{ directory })
	{
		if( ! f.is_regular_file() )
			continue;

		// skip files with wrong size
		size_t fileSize = f.file_size();
		if (fileSize < m_settings.minFileSize)
			continue;

		// check if the file is allowed in file masks
		const std::string& fpath = f.path().string();
		bool matched = false;
		for (regex& r : m_fileRegexes)
		{
			if (regex_match(fpath, r))
			{
				matched = true;
				break;
			}
		}
		if (!matched)
			continue;

		// insert to map { size -> set of file paths }
		if (auto it = res.find(fileSize); it == res.end())
			res[fileSize] = set<string>{ fpath };
		else
			it->second.insert(fpath);
	}
}