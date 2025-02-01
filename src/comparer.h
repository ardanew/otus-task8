#pragma once
#include <string>
#include <vector>
#include <set>
#include <cstdint>
#include "settings.h"
#include "hashes.h"

template<class Hasher = Md5, 
	class HashResult = typename std::result_of<decltype(&Hasher::calculate)(void*, size_t)>::type>
class Comparer
{
public:
	Comparer(const Settings& settings);
	std::vector<std::vector<std::string>> compare(size_t fileSize, std::set<std::string> files) const;

protected:
	const Settings &m_settings;
};
