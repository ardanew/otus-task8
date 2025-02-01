#pragma once
#include <cstdint>
#include <cstddef>

struct Crc32
{
	static uint32_t calculate(const void* data, size_t sz);
};

/// \brief struct to hold char[16] result of boost::md5 hash calculation
struct Md5_result
{
	uint64_t lo;
	uint64_t hi;
};
/// \brief comparison operator to allow md5 results be in set/map
bool operator<(const Md5_result& left, const Md5_result& right);

struct Md5
{
	static Md5_result calculate(const void* data, size_t sz);
};