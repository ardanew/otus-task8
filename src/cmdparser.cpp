#include "cmdparser.h"
#include <boost/program_options.hpp>
#include <sstream>
#include <filesystem>
namespace po = boost::program_options;
namespace fs = std::filesystem;
using namespace std;

tuple<Settings, string> CmdParser::parse(int argc, char **argv)
{
	try {
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "print help")
			("include,I", po::value<vector<string>>()->multitoken()->default_value({"."}, {"."}), "directories to include, space-separated")
			("exclude,E", po::value<vector<string>>()->multitoken()->default_value({}, {"empty"}), "directories to exclude, space-separated")
			("masks,M", po::value<vector<string>>()->multitoken()->default_value({".*"}, {".*"}), "permitted file masks, regex, space-separated, case-insensitive")
			("scan-depth,D", po::value<int>()->default_value(1), "scan depth, 1=recursive, 0=top level only")
			("file-size,S", po::value<int>()->default_value(1), "minimum file size to scan")
			("block-size,B", po::value<int>()->default_value(10), "read block size")
			("hash-type,H", po::value<int>()->default_value(0), "hash type, 0=CRC32, 1=MD5")
			;

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).
			options(desc).run(), vm);
		po::notify(vm);

		Settings settings;

		if (argc < 2 || vm.count("help") > 0)
		{
			settings.justPrintHelp = true;
			std::stringstream ss;
			ss << desc;
			return make_tuple(settings, ss.str());
		}

		settings.includeDirectories = vm["include"].as<vector<string>>();
		settings.excludeDirectories = vm["exclude"].as<vector<string>>();
		settings.enabledFileMasks = vm["masks"].as<vector<string>>();

		{
			int level = vm["scan-depth"].as<int>();
			if ( level == 1)
				settings.scanDepth = Settings::ScanDepth::RECURSIVE;
			else if (level == 0)
				settings.scanDepth = Settings::ScanDepth::TOPLEVEL;
			else
				throw std::runtime_error("bad scan depth argument");
		}	
		
		{
			int sz = vm["file-size"].as<int>();
			if( sz < 0 )
				throw std::runtime_error("bad file size argument");
			settings.minFileSize = sz;
		}

		{
			int sz = vm["block-size"].as<int>();
			if (sz < 0)
				throw runtime_error("bad block size argument");
			settings.readBlockSize = sz;
		}

		{
			int hashType = vm["hash-type"].as<int>();
			if (hashType == 0)
				settings.hashType = Settings::HashType::CRC32;
			else if (hashType == 1)
				settings.hashType = Settings::HashType::MD5;
			else
				throw runtime_error("bad hash type argument");
		}

		// convert potentially wrong slashes to system ones
		for (string& s : settings.includeDirectories)
			s = fs::path(s).make_preferred().string();
		for (string& s : settings.excludeDirectories)
			s = fs::path(s).make_preferred().string();

		return std::make_tuple(std::move(settings), "");
	}
	catch (exception &e) {
		Settings settings;
		settings.justPrintHelp = true;
		return std::make_tuple(std::move(settings), std::string("Exception ") + e.what() + "\nUse --help to show options\n");
	}
}