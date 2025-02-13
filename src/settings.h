#pragma once
#include <vector>
#include <string>

// to make some functions virtual when they are mocked in tests
// there is identical define in gmock, but for now we are not using it
#ifdef VIRTUAL_TESTS
#define VIRTUAL_4TESTS virtual
#else
#define VIRTUAL_4TESTS
#endif

/// \brief Holds settings from command line
struct Settings
{
	// implementation will set this to true when it is required just to show help and quit
	bool justPrintHelp = false;

	std::vector<std::string> includeDirectories = { {"."} };
	std::vector<std::string> excludeDirectories;
	enum class ScanDepth { RECURSIVE = 0, TOPLEVEL = 1 };
	ScanDepth scanDepth = ScanDepth::RECURSIVE;
	size_t minFileSize = 1;
	std::vector<std::string> enabledFileMasks = { {".*"} }; // case-insensitive
	size_t readBlockSize = 10;
	enum class HashType { CRC32, MD5 };
	HashType hashType = HashType::CRC32;
};