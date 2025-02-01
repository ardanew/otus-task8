#include "main.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include "cmdparser.h"
#include "walker.h"
#include "comparer.h"

using namespace std;

void printHelp(const char *executableName, const std::string &paramsDesc)
{
	cout << "Find duplicate (equal by hash) files" << endl;
	cout << "Usage: " << boost::filesystem::path(executableName).filename() << " [options]" << endl;
	cout << paramsDesc << endl;
}

int main(int argc, char **argv)
{
	// make settings from command line
	auto [settings, paramsDesc] = CmdParser::parse(argc, argv);
	if( settings.justPrintHelp )
	{
		printHelp(argv[0], paramsDesc);
		return 0;
	}

	// find files with same size
	Walker w{ settings };
	Walker::results_type candidates = w.run(); 

	// есть разные решения для изменения поведения при переключении хешей
	// например - сделать возвращаемое значение хеш-функций одинаковым для всех хешеров (string? uint128_t, был бы он?)
	// или сделать виртуальные функции
	// но мне они не понравились, хочется скорости. поэтому сделал хешеры с разными возвращаемыми типами
	// чтоб их использовать - не придумал ничего лучше чем вот такое ветвление
	// "как только начинается оптимизация - пропадает красота"	
	// впринципе то что под if можно закопать в отдельную шаблонную функцию, но пусть пока так
	if (settings.hashType == Settings::HashType::CRC32)
	{
		Comparer comp{ settings };
		for (auto& kvp : candidates)
		{
			vector<vector<string>> names = comp.compare(kvp.first, kvp.second);
			for (auto& group : names)
			{
				for (string& f : group)
					cout << f << endl;
				cout << endl;
			}
		}
	}
	else
	{
		Comparer<Md5> comp{ settings };
		for (auto& kvp : candidates)
		{
			vector<vector<string>> names = comp.compare(kvp.first, kvp.second);
			for (auto& group : names)
			{
				for (string& f : group)
					cout << f << endl;
				cout << endl;
			}
		}
	}

    return 0;
}