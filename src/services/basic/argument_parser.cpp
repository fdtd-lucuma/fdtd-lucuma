// Una GUI para fdtd
// Copyright © 2025 Otreblan
//
// fdtd-lucuma is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// fdtd-lucuma is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fdtd-lucuma.  If not, see <http://www.gnu.org/licenses/>.

module;

#include <getopt.h>
#include <cstdlib>

module lucuma.services.basic;

import std;
import magic_enum;

namespace lucuma::services::basic
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

std::optional<unsigned int> ArgumentParser::time() const
{
	return _time;
}

std::optional<Backend> ArgumentParser::backend() const
{
	return _backend;
}

std::optional<Precision> ArgumentParser::precision() const
{
	return _precision;
}

std::optional<SaveAs> ArgumentParser::saveAs() const
{
	return _saveAs;
}

void ArgumentParser::usage(int exit_code)
{
	std::print(
		"Usage: {} [options]...\n"
		"\t-h, --help         Show this help and exit.\n"
		"\t-H, --headless     Start as headless.\n"
		"\t-g, --no-headless  Start with gui.\n"
		"\t-G, --graph=FILE   Prints the services dependencies as a DAG in FILE.\n"
		"\t-x, --size-x=N     Set size x [default={}].\n"
		"\t-y, --size-y=N     Set size y [default={}].\n"
		"\t-z, --size-z=N     Set size z [default={}].\n"
		"\t-t, --time=N       Set simulation time steps [default={}].\n"
		"\t-b, --backend=NAME When running in headless mode use this backend [default={:?}].\n"
		"\t                   Values: {}.\n"
		"\t-p, --precision=fN Floating point precision as N bits [default={:?}].\n"
		"\t                   Values: {}.\n"
		"\t-s, --save-as=NAME Save as [default={:?}].\n"
		"\t                   Values: {}.\n",
		argv0(),
		Settings::defaultSizeX,
		Settings::defaultSizeY,
		Settings::defaultSizeZ,
		Settings::defaultTime,
		Settings::defaultBackend,
		magic_enum::enum_values<Backend>(),
		Settings::defaultPrecision,
		magic_enum::enum_values<Precision>(),
		Settings::defaultSaveAs,
		magic_enum::enum_values<SaveAs>()
	);

	exit(exit_code);
}

enum class Argument: int
{
	failure = '?',

	help        = 'h',
	headless    = 'H',
	no_headless = 'g',
	graph       = 'G',
	size_x      = 'x',
	size_y      = 'y',
	size_z      = 'z',
	time        = 't',
	backend     = 'b',
	precision   = 'p',
	save_as     = 's',
};

void ArgumentParser::parse(int argc, char** argv)
{
	int c;
	static const char shortopts[] = "hHgG:x:y:z:t:b:p:s:";
	static const option options[] {
		{"help",        no_argument,       nullptr, (int)Argument::help},
		{"headless",    no_argument,       nullptr, (int)Argument::headless},
		{"no-headless", no_argument,       nullptr, (int)Argument::no_headless},
		{"graph",       required_argument, nullptr, (int)Argument::graph},
		{"size-x",      required_argument, nullptr, (int)Argument::size_x},
		{"size-y",      required_argument, nullptr, (int)Argument::size_y},
		{"size-z",      required_argument, nullptr, (int)Argument::size_z},
		{"time",        required_argument, nullptr, (int)Argument::time},
		{"backend",     required_argument, nullptr, (int)Argument::backend},
		{"precision",   required_argument, nullptr, (int)Argument::precision},
		{"save_as",     required_argument, nullptr, (int)Argument::save_as},
		{nullptr,       0,                 nullptr, 0},
	};

	while((c = getopt_long(argc, argv, shortopts, options, nullptr)) != -1)
	{
		handleOption(c);
	}

	_positionalArguments.append_range(std::span(argv, argc).subspan(optind));
}

void ArgumentParser::handleOption(int shortopt)
{
	switch((Argument)shortopt)
	{
		case Argument::help:
			usage(EXIT_SUCCESS);
			std::unreachable();

		case Argument::headless:
			_isHeadless = true;
			break;

		case Argument::no_headless:
			_isHeadless = false;
			break;

		case Argument::graph:
			_graphPath.emplace(optarg);
			break;

		case Argument::size_x:
			fromString(_sizeX, optarg);
			break;

		case Argument::size_y:
			fromString(_sizeY, optarg);
			break;

		case Argument::size_z:
			fromString(_sizeZ, optarg);
			break;

		case Argument::time:
			fromString(_time, optarg);
			break;

		case Argument::backend:
			fromString(_backend, optarg);
			break;

		case Argument::precision:
			fromString(_precision, optarg);
			break;

		case Argument::save_as:
			fromString(_saveAs, optarg);
			break;

		case Argument::failure:
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
