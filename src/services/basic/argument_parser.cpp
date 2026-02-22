// Una GUI para fdtd
// Copyright © 2025-2026 Otreblan
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

bool ArgumentParser::debug() const
{
	return _debug;
}

bool ArgumentParser::tracy() const
{
	return _tracy;
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

std::optional<std::size_t> ArgumentParser::gaussX() const
{
	return _gaussX;
}

std::optional<std::size_t> ArgumentParser::gaussY() const
{
	return _gaussY;
}

std::optional<std::size_t> ArgumentParser::gaussZ() const
{
	return _gaussZ;
}

std::optional<float> ArgumentParser::deltaT() const
{
	return _deltaT;
}

std::optional<float> ArgumentParser::deltaX() const
{
	return _deltaX;
}

std::optional<float> ArgumentParser::deltaY() const
{
	return _deltaY;
}

std::optional<float> ArgumentParser::deltaZ() const
{
	return _deltaZ;
}

std::optional<float> ArgumentParser::imp0() const
{
	return _imp0;
}

std::optional<float> ArgumentParser::Cr() const
{
	return _Cr;
}

std::optional<float> ArgumentParser::gaussSigma() const
{
	return _gaussSigma;
}

const std::optional<std::filesystem::path>& ArgumentParser::Hx0() const
{
	return _Hx0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Hy0() const
{
	return _Hy0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Hz0() const
{
	return _Hz0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Chxh0() const
{
	return _Chxh0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Chyh0() const
{
	return _Chyh0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Chzh0() const
{
	return _Chzh0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Chxe0() const
{
	return _Chxe0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Chye0() const
{
	return _Chye0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Chze0() const
{
	return _Chze0;
}

const std::optional<std::filesystem::path>& ArgumentParser::CMhx0() const
{
	return _CMhx0;
}

const std::optional<std::filesystem::path>& ArgumentParser::CMhy0() const
{
	return _CMhy0;
}

const std::optional<std::filesystem::path>& ArgumentParser::CMhz0() const
{
	return _CMhz0;
}

const std::optional<std::filesystem::path>& ArgumentParser::mux0() const
{
	return _mux0;
}

const std::optional<std::filesystem::path>& ArgumentParser::muy0() const
{
	return _muy0;
}

const std::optional<std::filesystem::path>& ArgumentParser::muz0() const
{
	return _muz0;
}

const std::optional<std::filesystem::path>& ArgumentParser::muxR0() const
{
	return _muxR0;
}

const std::optional<std::filesystem::path>& ArgumentParser::muyR0() const
{
	return _muyR0;
}

const std::optional<std::filesystem::path>& ArgumentParser::muzR0() const
{
	return _muzR0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Ex0() const
{
	return _Ex0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Ey0() const
{
	return _Ey0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Ez0() const
{
	return _Ez0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Cexe0() const
{
	return _Cexe0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Ceye0() const
{
	return _Ceye0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Ceze0() const
{
	return _Ceze0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Cexh0() const
{
	return _Cexh0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Ceyh0() const
{
	return _Ceyh0;
}

const std::optional<std::filesystem::path>& ArgumentParser::Cezh0() const
{
	return _Cezh0;
}

const std::optional<std::filesystem::path>& ArgumentParser::CEEx0() const
{
	return _CEEx0;
}

const std::optional<std::filesystem::path>& ArgumentParser::CEEy0() const
{
	return _CEEy0;
}

const std::optional<std::filesystem::path>& ArgumentParser::CEEz0() const
{
	return _CEEz0;
}

const std::optional<std::filesystem::path>& ArgumentParser::epsx0() const
{
	return _epsx0;
}

const std::optional<std::filesystem::path>& ArgumentParser::epsy0() const
{
	return _epsy0;
}

const std::optional<std::filesystem::path>& ArgumentParser::epsz0() const
{
	return _epsz0;
}

const std::optional<std::filesystem::path>& ArgumentParser::epsxR0() const
{
	return _epsxR0;
}

const std::optional<std::filesystem::path>& ArgumentParser::epsyR0() const
{
	return _epsyR0;
}

const std::optional<std::filesystem::path>& ArgumentParser::epszR0() const
{
	return _epszR0;
}

const std::optional<std::filesystem::path>& ArgumentParser::eyx00() const
{
	return _eyx00;
}

const std::optional<std::filesystem::path>& ArgumentParser::ezx00() const
{
	return _ezx00;
}

const std::optional<std::filesystem::path>& ArgumentParser::eyx10() const
{
	return _eyx10;
}

const std::optional<std::filesystem::path>& ArgumentParser::ezx10() const
{
	return _ezx10;
}

const std::optional<std::filesystem::path>& ArgumentParser::exy00() const
{
	return _exy00;
}

const std::optional<std::filesystem::path>& ArgumentParser::ezy00() const
{
	return _ezy00;
}

const std::optional<std::filesystem::path>& ArgumentParser::exy10() const
{
	return _exy10;
}

const std::optional<std::filesystem::path>& ArgumentParser::ezy10() const
{
	return _ezy10;
}

const std::optional<std::filesystem::path>& ArgumentParser::exz00() const
{
	return _exz00;
}

const std::optional<std::filesystem::path>& ArgumentParser::eyz00() const
{
	return _eyz00;
}

const std::optional<std::filesystem::path>& ArgumentParser::exz10() const
{
	return _exz10;
}

const std::optional<std::filesystem::path>& ArgumentParser::eyz10() const
{
	return _eyz10;
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
		"\t-h, --help          Show this help and exit.\n"
		"\t-H, --headless      Start as headless.\n"
		"\t-g, --no-headless   Start with gui.\n"
		"\t-G, --graph=FILE    Prints the services dependencies as a DAG in FILE.\n"
		"\t-x, --size-x=N      Set size x [default={}].\n"
		"\t-y, --size-y=N      Set size y [default={}].\n"
		"\t-z, --size-z=N      Set size z [default={}].\n"
		"\t-t, --time=N        Set simulation time steps [default={}].\n"
		"\t-b, --backend=NAME  When running in headless mode use this backend [default={:?}].\n"
		"\t                    Values: {}.\n"
		"\t-p, --precision=fN  Floating point precision as N bits [default={:?}].\n"
		"\t                    Values: {}.\n"
		"\t-s, --save-as=NAME  Save as [default={:?}].\n"
		"\t                    Values: {}.\n"
		"\t-d, --debug         Print debug values.\n"
		"\t-D, --no-debug      Don't print debug values.\n"
		"\t-i, --gauss-x=N     Set gauss origin X.\n"
		"\t-j, --gauss-y=N     Set gauss origin Y.\n"
		"\t-k, --gauss-z=N     Set gauss origin Z.\n"
		"\t-T, --delta-t=N     Set deltaT.\n"
		"\t-X, --delta-x=N     Set deltaX.\n"
		"\t-Y, --delta-y=N     Set deltaY.\n"
		"\t-Z, --delta-z=N     Set deltaZ.\n"
		"\t-I, --imp0=N        Set impedance.\n"
		"\t-c, --Cr=N          Set Courant number.\n"
		"\t-S, --gauss-sigma=N Set gaussian sigma.\n"
		"\t--Hx0=PATH          Text file with initial values.\n"
		"\t--Hy0=PATH          Text file with initial values.\n"
		"\t--Hz0=PATH          Text file with initial values.\n"
		"\t--Chxh0=PATH        Text file with initial values.\n"
		"\t--Chyh0=PATH        Text file with initial values.\n"
		"\t--Chzh0=PATH        Text file with initial values.\n"
		"\t--Chxe0=PATH        Text file with initial values.\n"
		"\t--Chye0=PATH        Text file with initial values.\n"
		"\t--Chze0=PATH        Text file with initial values.\n"
		"\t--CMhx0=PATH        Text file with initial values.\n"
		"\t--CMhy0=PATH        Text file with initial values.\n"
		"\t--CMhz0=PATH        Text file with initial values.\n"
		"\t--mux0=PATH         Text file with initial values.\n"
		"\t--muy0=PATH         Text file with initial values.\n"
		"\t--muz0=PATH         Text file with initial values.\n"
		"\t--muxR0=PATH        Text file with initial values.\n"
		"\t--muyR0=PATH        Text file with initial values.\n"
		"\t--muzR0=PATH        Text file with initial values.\n"
		"\t--Ex0=PATH          Text file with initial values.\n"
		"\t--Ey0=PATH          Text file with initial values.\n"
		"\t--Ez0=PATH          Text file with initial values.\n"
		"\t--Cexe0=PATH        Text file with initial values.\n"
		"\t--Ceye0=PATH        Text file with initial values.\n"
		"\t--Ceze0=PATH        Text file with initial values.\n"
		"\t--Cexh0=PATH        Text file with initial values.\n"
		"\t--Ceyh0=PATH        Text file with initial values.\n"
		"\t--Cezh0=PATH        Text file with initial values.\n"
		"\t--CEEx0=PATH        Text file with initial values.\n"
		"\t--CEEy0=PATH        Text file with initial values.\n"
		"\t--CEEz0=PATH        Text file with initial values.\n"
		"\t--epsx0=PATH        Text file with initial values.\n"
		"\t--epsy0=PATH        Text file with initial values.\n"
		"\t--epsz0=PATH        Text file with initial values.\n"
		"\t--epsxR0=PATH       Text file with initial values.\n"
		"\t--epsyR0=PATH       Text file with initial values.\n"
		"\t--epszR0=PATH       Text file with initial values.\n"
		"\t--eyx00=PATH        Text file with initial values.\n"
		"\t--ezx00=PATH        Text file with initial values.\n"
		"\t--eyx10=PATH        Text file with initial values.\n"
		"\t--ezx10=PATH        Text file with initial values.\n"
		"\t--exy00=PATH        Text file with initial values.\n"
		"\t--ezy00=PATH        Text file with initial values.\n"
		"\t--exy10=PATH        Text file with initial values.\n"
		"\t--ezy10=PATH        Text file with initial values.\n"
		"\t--exz00=PATH        Text file with initial values.\n"
		"\t--eyz00=PATH        Text file with initial values.\n"
		"\t--exz10=PATH        Text file with initial values.\n"
		"\t--eyz10=PATH        Text file with initial values.\n"
		"\t--tracy             Enable tracy profiling.\n"
		"\t--no-tracy          Disable tracy profiling.\n"
		,
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
	debug       = 'd',
	no_debug    = 'D',
	gauss_x     = 'i',
	gauss_y     = 'j',
	gauss_z     = 'k',
	delta_t     = 'T',
	delta_x     = 'X',
	delta_y     = 'Y',
	delta_z     = 'Z',
	imp0        = 'I',
	Cr          = 'c',
	gauss_sigma = 'S',

	separator = 256,

	Hx0,
	Hy0,
	Hz0,
	Chxh0,
	Chyh0,
	Chzh0,
	Chxe0,
	Chye0,
	Chze0,
	CMhx0,
	CMhy0,
	CMhz0,
	mux0,
	muy0,
	muz0,
	muxR0,
	muyR0,
	muzR0,
	Ex0,
	Ey0,
	Ez0,
	Cexe0,
	Ceye0,
	Ceze0,
	Cexh0,
	Ceyh0,
	Cezh0,
	CEEx0,
	CEEy0,
	CEEz0,
	epsx0,
	epsy0,
	epsz0,
	epsxR0,
	epsyR0,
	epszR0,
	eyx00,
	ezx00,
	eyx10,
	ezx10,
	exy00,
	ezy00,
	exy10,
	ezy10,
	exz00,
	eyz00,
	exz10,
	eyz10,

	tracy,
	no_tracy,

};

void ArgumentParser::parse(int argc, char** argv)
{
	int c;
	static const char shortopts[] = "hHgG:x:y:z:t:b:p:s:dDi:j:k:T:X:Y:Z:I:c:S:";
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
		{"save-as",     required_argument, nullptr, (int)Argument::save_as},
		{"debug",       no_argument,       nullptr, (int)Argument::debug},
		{"no-debug",    no_argument,       nullptr, (int)Argument::no_debug},
		{"gauss-x",     required_argument, nullptr, (int)Argument::gauss_x},
		{"gauss-y",     required_argument, nullptr, (int)Argument::gauss_y},
		{"gauss-z",     required_argument, nullptr, (int)Argument::gauss_z},
		{"delta-t",     required_argument, nullptr, (int)Argument::delta_t},
		{"delta-x",     required_argument, nullptr, (int)Argument::delta_x},
		{"delta-y",     required_argument, nullptr, (int)Argument::delta_y},
		{"delta-z",     required_argument, nullptr, (int)Argument::delta_z},
		{"imp0",        required_argument, nullptr, (int)Argument::imp0},
		{"Cr",          required_argument, nullptr, (int)Argument::Cr},
		{"gauss-sigma", required_argument, nullptr, (int)Argument::gauss_sigma},
		{"Hx0",         required_argument, nullptr, (int)Argument::Hx0},
		{"Hy0",         required_argument, nullptr, (int)Argument::Hy0},
		{"Hz0",         required_argument, nullptr, (int)Argument::Hz0},
		{"Chxh0",       required_argument, nullptr, (int)Argument::Chxh0},
		{"Chyh0",       required_argument, nullptr, (int)Argument::Chyh0},
		{"Chzh0",       required_argument, nullptr, (int)Argument::Chzh0},
		{"Chxe0",       required_argument, nullptr, (int)Argument::Chxe0},
		{"Chye0",       required_argument, nullptr, (int)Argument::Chye0},
		{"Chze0",       required_argument, nullptr, (int)Argument::Chze0},
		{"CMhx0",       required_argument, nullptr, (int)Argument::CMhx0},
		{"CMhy0",       required_argument, nullptr, (int)Argument::CMhy0},
		{"CMhz0",       required_argument, nullptr, (int)Argument::CMhz0},
		{"mux0",        required_argument, nullptr, (int)Argument::mux0},
		{"muy0",        required_argument, nullptr, (int)Argument::muy0},
		{"muz0",        required_argument, nullptr, (int)Argument::muz0},
		{"muxR0",       required_argument, nullptr, (int)Argument::muxR0},
		{"muyR0",       required_argument, nullptr, (int)Argument::muyR0},
		{"muzR0",       required_argument, nullptr, (int)Argument::muzR0},
		{"Ex0",         required_argument, nullptr, (int)Argument::Ex0},
		{"Ey0",         required_argument, nullptr, (int)Argument::Ey0},
		{"Ez0",         required_argument, nullptr, (int)Argument::Ez0},
		{"Cexe0",       required_argument, nullptr, (int)Argument::Cexe0},
		{"Ceye0",       required_argument, nullptr, (int)Argument::Ceye0},
		{"Ceze0",       required_argument, nullptr, (int)Argument::Ceze0},
		{"Cexh0",       required_argument, nullptr, (int)Argument::Cexh0},
		{"Ceyh0",       required_argument, nullptr, (int)Argument::Ceyh0},
		{"Cezh0",       required_argument, nullptr, (int)Argument::Cezh0},
		{"CEEx0",       required_argument, nullptr, (int)Argument::CEEx0},
		{"CEEy0",       required_argument, nullptr, (int)Argument::CEEy0},
		{"CEEz0",       required_argument, nullptr, (int)Argument::CEEz0},
		{"epsx0",       required_argument, nullptr, (int)Argument::epsx0},
		{"epsy0",       required_argument, nullptr, (int)Argument::epsy0},
		{"epsz0",       required_argument, nullptr, (int)Argument::epsz0},
		{"epsxR0",      required_argument, nullptr, (int)Argument::epsxR0},
		{"epsyR0",      required_argument, nullptr, (int)Argument::epsyR0},
		{"epszR0",      required_argument, nullptr, (int)Argument::epszR0},
		{"eyx00",       required_argument, nullptr, (int)Argument::eyx00},
		{"ezx00",       required_argument, nullptr, (int)Argument::ezx00},
		{"eyx10",       required_argument, nullptr, (int)Argument::eyx10},
		{"ezx10",       required_argument, nullptr, (int)Argument::ezx10},
		{"exy00",       required_argument, nullptr, (int)Argument::exy00},
		{"ezy00",       required_argument, nullptr, (int)Argument::ezy00},
		{"exy10",       required_argument, nullptr, (int)Argument::exy10},
		{"ezy10",       required_argument, nullptr, (int)Argument::ezy10},
		{"exz00",       required_argument, nullptr, (int)Argument::exz00},
		{"eyz00",       required_argument, nullptr, (int)Argument::eyz00},
		{"exz10",       required_argument, nullptr, (int)Argument::exz10},
		{"eyz10",       required_argument, nullptr, (int)Argument::eyz10},
		{"tracy",       required_argument, nullptr, (int)Argument::tracy},
		{"no-tracy",    required_argument, nullptr, (int)Argument::no_tracy},

		{nullptr,       0,                 nullptr, 0},
	};

	if(argc == 1)
		_isHeadless = false;

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

		case Argument::debug:
			_debug = true;
			break;

		case Argument::no_debug:
			_debug = false;
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

		case Argument::gauss_x:
			fromString(_gaussX, optarg);
			break;

		case Argument::gauss_y:
			fromString(_gaussY, optarg);
			break;

		case Argument::gauss_z:
			fromString(_gaussZ, optarg);
			break;

		case Argument::delta_t:
			fromString(_deltaT, optarg);
			break;

		case Argument::delta_x:
			fromString(_deltaX, optarg);
			break;

		case Argument::delta_y:
			fromString(_deltaY, optarg);
			break;

		case Argument::delta_z:
			fromString(_deltaZ, optarg);
			break;

		case Argument::imp0:
			fromString(_imp0, optarg);
			break;

		case Argument::Cr:
			fromString(_Cr, optarg);
			break;

		case Argument::gauss_sigma:
			fromString(_gaussSigma, optarg);
			break;

		case Argument::Hx0:
			fromString(_Hx0, optarg);
			break;

		case Argument::Hy0:
			fromString(_Hy0, optarg);
			break;

		case Argument::Hz0:
			fromString(_Hz0, optarg);
			break;

		case Argument::Chxh0:
			fromString(_Chxh0, optarg);
			break;

		case Argument::Chyh0:
			fromString(_Chyh0, optarg);
			break;

		case Argument::Chzh0:
			fromString(_Chzh0, optarg);
			break;

		case Argument::Chxe0:
			fromString(_Chxe0, optarg);
			break;

		case Argument::Chye0:
			fromString(_Chye0, optarg);
			break;

		case Argument::Chze0:
			fromString(_Chze0, optarg);
			break;

		case Argument::CMhx0:
			fromString(_CMhx0, optarg);
			break;

		case Argument::CMhy0:
			fromString(_CMhy0, optarg);
			break;

		case Argument::CMhz0:
			fromString(_CMhz0, optarg);
			break;

		case Argument::mux0:
			fromString(_mux0, optarg);
			break;

		case Argument::muy0:
			fromString(_muy0, optarg);
			break;

		case Argument::muz0:
			fromString(_muz0, optarg);
			break;

		case Argument::muxR0:
			fromString(_muxR0, optarg);
			break;

		case Argument::muyR0:
			fromString(_muyR0, optarg);
			break;

		case Argument::muzR0:
			fromString(_muzR0, optarg);
			break;

		case Argument::Ex0:
			fromString(_Ex0, optarg);
			break;

		case Argument::Ey0:
			fromString(_Ey0, optarg);
			break;

		case Argument::Ez0:
			fromString(_Ez0, optarg);
			break;

		case Argument::Cexe0:
			fromString(_Cexe0, optarg);
			break;

		case Argument::Ceye0:
			fromString(_Ceye0, optarg);
			break;

		case Argument::Ceze0:
			fromString(_Ceze0, optarg);
			break;

		case Argument::Cexh0:
			fromString(_Cexh0, optarg);
			break;

		case Argument::Ceyh0:
			fromString(_Ceyh0, optarg);
			break;

		case Argument::Cezh0:
			fromString(_Cezh0, optarg);
			break;

		case Argument::CEEx0:
			fromString(_CEEx0, optarg);
			break;

		case Argument::CEEy0:
			fromString(_CEEy0, optarg);
			break;

		case Argument::CEEz0:
			fromString(_CEEz0, optarg);
			break;

		case Argument::epsx0:
			fromString(_epsx0, optarg);
			break;

		case Argument::epsy0:
			fromString(_epsy0, optarg);
			break;

		case Argument::epsz0:
			fromString(_epsz0, optarg);
			break;

		case Argument::epsxR0:
			fromString(_epsxR0, optarg);
			break;

		case Argument::epsyR0:
			fromString(_epsyR0, optarg);
			break;

		case Argument::epszR0:
			fromString(_epszR0, optarg);
			break;

		case Argument::eyx00:
			fromString(_eyx00, optarg);
			break;

		case Argument::ezx00:
			fromString(_ezx00, optarg);
			break;

		case Argument::eyx10:
			fromString(_eyx10, optarg);
			break;

		case Argument::ezx10:
			fromString(_ezx10, optarg);
			break;

		case Argument::exy00:
			fromString(_exy00, optarg);
			break;

		case Argument::ezy00:
			fromString(_ezy00, optarg);
			break;

		case Argument::exy10:
			fromString(_exy10, optarg);
			break;

		case Argument::ezy10:
			fromString(_ezy10, optarg);
			break;

		case Argument::exz00:
			fromString(_exz00, optarg);
			break;

		case Argument::eyz00:
			fromString(_eyz00, optarg);
			break;

		case Argument::exz10:
			fromString(_exz10, optarg);
			break;

		case Argument::eyz10:
			fromString(_eyz10, optarg);
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

		case Argument::tracy:
			_tracy = true;
			break;

		case Argument::no_tracy:
			_tracy = false;
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
