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
	_argv0(argv[0])
{
	parse(argc, argv);
}

std::string_view ArgumentParser::argv0() const
{
	return _argv0;
}

std::span<const std::string> ArgumentParser::positionalArguments() const
{
	return _positionalArguments;
}


bool ArgumentParser::isHeadless() const
{
	return _isHeadless;
}

const std::optional<std::filesystem::path>& ArgumentParser::graphPath() const
{
	return _graphPath;
}

std::optional<std::size_t> ArgumentParser::sizeX() const
{
	return _sizeX;
}

std::optional<std::size_t> ArgumentParser::sizeY() const
{
	return _sizeY;
}

std::optional<std::size_t> ArgumentParser::sizeZ() const
{
	return _sizeZ;
}

void ArgumentParser::usage(int exit_code)
{
	std::print(
		"Usage: {} [options]...\n"
		"\t-h, --help        Show this help and exit.\n"
		"\t-H, --headless    Start as headless.\n"
		"\t-g, --no-headless Start with gui.\n"
		"\t-G, --graph=FILE  Prints the services dependencies as a DAG in FILE.\n"
		"\t-x, --size-x=N    Set size x [default={}].\n"
		"\t-y, --size-y=N    Set size y [default={}].\n"
		"\t-z, --size-z=N    Set size z [default={}].\n",
		argv0(),
		Settings::defaultSizeX,
		Settings::defaultSizeY,
		Settings::defaultSizeZ
	);

	exit(exit_code);
}

void ArgumentParser::parse(int argc, char** argv)
{
	int c;
	static const char shortopts[] = "hHgG:x:y:z:";
	static const option options[] {
		{"help",        no_argument,       nullptr, 'h'},
		{"headless",    no_argument,       nullptr, 'H'},
		{"no-headless", no_argument,       nullptr, 'g'},
		{"graph",       required_argument, nullptr, 'G'},
		{"size-x",      required_argument, nullptr, 'x'},
		{"size-y",      required_argument, nullptr, 'y'},
		{"size-z",      required_argument, nullptr, 'z'},
		{nullptr,       0,                 nullptr, 0},
	};

	while((c = getopt_long(argc, argv, shortopts, options, nullptr)) != -1)
	{
		handleOption(c);
	}

	for(int i = optind; i < argc; i++)
	{
		_positionalArguments.emplace_back(argv[i]);
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

		case 'x': // --size-x
			_sizeX.emplace(fromString<std::size_t>(optarg));
			break;

		case 'y': // --size-y
			_sizeY.emplace(fromString<std::size_t>(optarg));
			break;

		case 'z': // --size-z
			_sizeZ.emplace(fromString<std::size_t>(optarg));
			break;

		case '?':
			usage(EXIT_FAILURE);
			std::unreachable();

		default:
			// Nothing
	}
}

void ArgumentParser::fail(std::string_view str, std::errc ec)
{
	std::println(std::cerr, "{}: {}", str, std::make_error_code(ec).message());

	exit(EXIT_FAILURE);
}

}
