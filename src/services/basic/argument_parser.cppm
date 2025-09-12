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

export module fdtd.services.basic:argument_parser;

import fdtd.utils;
import std;

namespace fdtd::services::basic
{

export class ArgumentParser
{
public:
	ArgumentParser(int argc, char** argv);

	std::string_view             getArgv0() const;
	std::span<const std::string> getPositionalArguments() const;

	bool isHeadless() const;

private:
	std::string              argv0;
	std::vector<std::string> positionalArguments;

	bool _isHeadless = true;

	[[noreturn]]
	void usage(int exit_code);

	void parse(int argc, char** argv);

	void handleOption(char shortopt);
};

}
