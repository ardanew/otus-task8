#include "hashes.h"
#include <boost/crc.hpp>
#include <boost/uuid/detail/md5.hpp>

uint32_t Crc32::calculate(const void* data, size_t sz)
{
	boost::crc_32_type result;
	result.process_bytes(data, sz);
	return result.checksum();

}

bool operator<(const Md5_result& left, const Md5_result& right)
{
	if (left.hi < right.hi)
		return true;
	else if (left.hi > right.hi)
		return false;
	return left.lo < right.lo;
}

Md5_result Md5::calculate(const void* data, size_t sz)
{
	using boost::uuids::detail::md5;

	md5 hash;
	hash.process_bytes(data, sz);

	// эта штука возвращает char[16], а его неудобно в мапу совать
	// сделаю чтоб возвращало результат в мой тип
	md5::digest_type digest;
	hash.get_digest(digest);
	return *reinterpret_cast<Md5_result*>(&digest);
}