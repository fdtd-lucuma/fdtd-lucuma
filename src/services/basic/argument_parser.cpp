// Una GUI para fdtd
// Copyright © 2025 Otreblan
//
// fdtd-vulkan is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// fdtd-vulkan is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fdtd-vulkan.  If not, see <http://www.gnu.org/licenses/>.

module;

#include <getopt.h>
#include <cstdlib>

module fdtd.services.basic;

import std;

namespace fdtd::services::basic
{

ArgumentParser::ArgumentParser(int argc, char** argv):
	argv0(argv[0])
{
	parse(argc, argv);
}

std::string_view ArgumentParser::getArgv0() const
{
	return argv0;
}

std::span<const std::string> ArgumentParser::getPositionalArguments() const
{
	return positionalArguments;
}


bool ArgumentParser::isHeadless() const
{
	return _isHeadless;
}

const std::optional<std::filesystem::path>& ArgumentParser::graphPath() const
{
	return _graphPath;
}

void ArgumentParser::usage(int exit_code)
{
	std::cout
		<< "Usage: " << getArgv0() << " [options]...\n"
		<< "\t-h, --help        Show this help and exit.\n"
		<< "\t-H, --headless    Start as headless.\n"
		<< "\t-g, --no-headless Start with gui.\n"
		<< "\t-G, --graph=FILE  Prints the services dependencies as a DAG in FILE.\n"
	;

	exit(exit_code);
}

void ArgumentParser::parse(int argc, char** argv)
{
	int c;
	static const char shortopts[] = "hHgG:";
	static const option options[] {
		{"help",        no_argument,       nullptr, 'h'},
		{"headless",    no_argument,       nullptr, 'H'},
		{"no-headless", no_argument,       nullptr, 'g'},
		{"graph",       required_argument, nullptr, 'G'},
		{nullptr,       0,                 nullptr, 0},
	};

	while((c = getopt_long(argc, argv, shortopts, options, nullptr)) != -1)
	{
		handleOption(c);
	}

	for(int i = optind; i < argc; i++)
	{
		positionalArguments.emplace_back(argv[i]);
	}

}

void ArgumentParser::handleOption(char shortopt)
{
	switch(shortopt)
	{
		case 'h': // --help
			usage(EXIT_SUCCESS);
			std::unreachable();

		case 'H': // --headless
			_isHeadless = true;
			break;

		case 'g': // --no-headless
			_isHeadless = false;
			break;

		case 'G': // --graph
			_graphPath.emplace(optarg);
			break;

		case '?':
			usage(EXIT_FAILURE);
			std::unreachable();

		default:
			// Nothing
	}
}

}
