#pragma once
#include "settings.h"
#include <map>
#include <set>
#include <vector>
#include <regex>

/// \brief Find files with same size using settings
class Walker
{
public:
	Walker(const Settings& settings);

	using results_type = std::map<size_t, std::set<std::string>>;

	/// \brief scans for files with same size
	/// \return map file_size -> set of file paths
	results_type run();

protected:
	const Settings &m_settings;

	std::vector<std::regex> m_fileRegexes;

	/// \brief checks if dir contains path from exclude list
	bool isDirectoryExcluded(const std::string& dir) const;

	/// \brief adds all good files from directory to res
	/// VIRTUAL_4TESTS is defind to virtual in tests and to nothing in production
	VIRTUAL_4TESTS void parseFilesInDirectory(const std::string& directory, results_type &res);
};