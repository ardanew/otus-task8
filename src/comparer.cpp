#include "comparer.h"
#include <filesystem>
#include <map>
#include <cstring>
#include <fstream>
#include <cassert>
namespace fs = std::filesystem;
using namespace std;

template class Comparer<Crc32>;
template class Comparer<Md5>;

template<class Hasher, class HashResult>
Comparer<Hasher, HashResult>::Comparer(const Settings& settings) : m_settings(settings)
{
}

// NOTE � ���� ��� ���� ������� ����� hashmap
// �� ��� ���������� ������� �������
struct File
{
	File() = default;
	File(const std::string& n) : name(n), offset(0) {
		assert(fs::exists(name));
		stream = std::ifstream{ name, std::ios::binary };
	}
	std::ifstream stream;
	size_t offset;
	std::string name;
};

template<class Hasher, class HashResult>
vector<vector<string>> Comparer<Hasher, HashResult>::compare(size_t fileSize, std::set<std::string> files) const
{
	// ��� ��� ��������� ��-�� ���� ��� �� ���� ����� ��������� 4 �����
	// ������ ��� ������� ����� � ������ ��� �����

	// �� ���� ����� ����� ���� ������ �� 2� � ����� ������
	vector<vector<File>> input;
	{
		vector<File> candidates;
		for (const string& f : files)
			candidates.emplace_back(File(f));
		input.push_back(std::move(candidates));
	}

	// ����� ����� ���������� ������ �������
	vector<vector<File>> results;
	
	// ���� ����� ������ ����� �� ������
	std::vector<char> buf;
	buf.resize(m_settings.readBlockSize);

	// ���� ������� ��������� �� ��� ����� ��������, �� ���� ������������ �� ������� ������
	while (true)
	{
		if (input.begin()->begin()->offset >= fileSize)
		{ // ����� �� ����� ������, ���������� �� � ���������� �������
			for (vector<File>& f : input)
				results.push_back(std::move(f));
			break;
		}

		// �� ���� ����� �������� ������ ������
		// �� ������ �������� (�������� ��� �������) ����� ������ ������
		// � ���������� �� ���� ��������� �������� ����� ��� ������� ���������� �����

		vector<vector<File>> newInput; // ��� ����� �������� �� ��������� �������� �����

		// �������� �� �������
		for (vector<File>& candidatesGroup : input)
		{
			// ��� -> ����� � ���������� �����
			std::map<HashResult, std::vector<File>> tempResults;

			for (File& candidate : candidatesGroup)
			{
				// ��� ��������� ����� �������� ����� - ��� �������� ����� �� �� ����� (����� ������� ���� �����������)
				if (candidate.offset + m_settings.readBlockSize > fileSize)
					memset(&buf[0], 0, buf.size());

				// ������ ����
				candidate.stream.read(&buf[0], m_settings.readBlockSize);
				candidate.offset += m_settings.readBlockSize;

				// ������� ���
				HashResult hash = Hasher::calculate(&buf[0], buf.size());

				// ���������� � ���� [hash -> files with same hash]
				if (auto it = tempResults.find(hash); it == tempResults.end())
				{
					tempResults[hash] = vector<File>();
					tempResults[hash].push_back(std::move(candidate));
				}
				else
					it->second.push_back(std::move(candidate));
			}

			// �� ������ ���� � ������� ���� ���� - �� �����
			for (auto it = tempResults.begin(); it != tempResults.end(); )
			{
				if (it->second.size() <= 1)
					tempResults.erase(it++);
				else
					++it;
			}

			// ���������� ������ ���� - ��� ����� ������ ������
			for (auto& kvp : tempResults)
				newInput.push_back(std::move(kvp.second));
		}

		if (newInput.empty())
			break; // ��� ������� ����� ����������
		input = std::move(newInput);
	}

	// �� ����� ��������� ������ �����
	std::vector<std::vector<std::string>> out;
	for (auto& outer : results)
	{
		vector<string> innerVector;
		for (auto& inner : outer)
			innerVector.push_back(std::move(inner.name));
		out.push_back(std::move(innerVector));
	}
	return out;
}

