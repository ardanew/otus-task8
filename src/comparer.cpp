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

// NOTE € знаю что есть решение через hashmap
// но мне захотелось вручную сделать
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
	// все эти сложности из-за того что на вход могут прилететь 4 файла
	// первые два которых равны и вторые два равны

	// на вход цикла пойдЄт одна группа из 2х и более файлов
	vector<vector<File>> input;
	{
		vector<File> candidates;
		for (const string& f : files)
			candidates.emplace_back(File(f));
		input.push_back(std::move(candidates));
	}

	// здесь будут результаты работы функции
	vector<vector<File>> results;
	
	// сюда будем читать чанки из файлов
	std::vector<char> buf;
	buf.resize(m_settings.readBlockSize);

	// если сделать рекурсией то код будет красивый, но стек переполнитс€ на больших файлах
	while (true)
	{
		if (input.begin()->begin()->offset >= fileSize)
		{ // дошли до конца файлов, перемещаем их в результаты функции
			for (vector<File>& f : input)
				results.push_back(std::move(f));
			break;
		}

		// на вход цикла подаютс€ группы файлов
		// на выходе получаем (возможно ещЄ большую) новую группу файлов
		// еЄ отправл€ем на вход следующей итерации цикла дл€ обсчЄта следующего чанка

		vector<vector<File>> newInput; // это будем подавать на следующую итерацию цикла

		// проходим по группам
		for (vector<File>& candidatesGroup : input)
		{
			// хеш -> файлы с одинаковым хешем
			std::map<HashResult, std::vector<File>> tempResults;

			for (File& candidate : candidatesGroup)
			{
				// дл€ конечного чанка занул€ем буфер - ибо прочесть можем не до конца (можно сделать чуть оптимальней)
				if (candidate.offset + m_settings.readBlockSize > fileSize)
					memset(&buf[0], 0, buf.size());

				// читаем блок
				candidate.stream.read(&buf[0], m_settings.readBlockSize);
				candidate.offset += m_settings.readBlockSize;

				// считаем хеш
				HashResult hash = Hasher::calculate(&buf[0], buf.size());

				// складываем в мапу [hash -> files with same hash]
				if (auto it = tempResults.find(hash); it == tempResults.end())
				{
					tempResults[hash] = vector<File>();
					tempResults[hash].push_back(std::move(candidate));
				}
				else
					it->second.push_back(std::move(candidate));
			}

			// те €чейки мапы в который один файл - не нужны
			for (auto it = tempResults.begin(); it != tempResults.end(); )
			{
				if (it->second.size() <= 1)
					tempResults.erase(it++);
				else
					++it;
			}

			// оставшиес€ €чейки мапы - это новые группы файлов
			for (auto& kvp : tempResults)
				newInput.push_back(std::move(kvp.second));
		}

		if (newInput.empty())
			break; // все входные файлы отличаютс€
		input = std::move(newInput);
	}

	// на выход требуютс€ только имена
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

