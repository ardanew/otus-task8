#pragma once
#include <string>
#include <vector>
#include <set>
#include <cstdint>
#include "settings.h"
#include "hashes.h"

/// \brief Class to find duplicates
/// first template argument must be Crc32, Md5 or other hasher (if implemented)
template<class Hasher = Crc32, 
	class HashResult = typename std::result_of<decltype(&Hasher::calculate)(void*, size_t)>::type>
class Comparer
{
public:
	/// \brief creates a comparer object with settings
	Comparer(const Settings& settings);

	/// \brief parse files using template hasher
	/// \param fileSize size in bytes, identical for each file in files set
	/// \param files set of files with same size
	/// \return grouped file names, identical files are in one group
	std::vector<std::vector<std::string>> compare(size_t fileSize, std::set<std::string> files) const;

protected:
	const Settings &m_settings;
};
