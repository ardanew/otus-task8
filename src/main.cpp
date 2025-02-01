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

	// ���� ������ ������� ��� ��������� ��������� ��� ������������ �����
	// �������� - ������� ������������ �������� ���-������� ���������� ��� ���� ������� (string? uint128_t, ��� �� ��?)
	// ��� ������� ����������� �������
	// �� ��� ��� �� �����������, ������� ��������. ������� ������ ������ � ������� ������������� ������
	// ���� �� ������������ - �� �������� ������ ����� ��� ��� ����� ���������
	// "��� ������ ���������� ����������� - ��������� �������"	
	// ��������� �� ��� ��� if ����� �������� � ��������� ��������� �������, �� ����� ���� ���
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