#pragma once
#include <tuple>
#include "settings.h"

/// \brief class to parse command line
class CmdParser
{
public:
	/// \brief Parse specified command line
	/// \return First tuple item=parsed settings, second item=parameters description to display help
	static std::tuple<Settings, std::string> parse(int argc, char **argv);
};