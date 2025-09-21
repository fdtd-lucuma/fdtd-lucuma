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

export module lucuma.services.basic:argument_parser;

import :settings_enums;

import lucuma.utils;
import std;

namespace lucuma::services::basic
{

export class ArgumentParser
{
public:
	ArgumentParser(int argc, char** argv);

	std::string_view             argv0() const;
	std::span<const std::string> positionalArguments() const;

	bool isHeadless() const;
	const std::optional<std::filesystem::path>& graphPath() const;

	std::optional<std::size_t> sizeX() const;
	std::optional<std::size_t> sizeY() const;
	std::optional<std::size_t> sizeZ() const;

	std::optional<unsigned int> time() const;

	std::optional<Backend> backend() const;

private:
	std::string              _argv0;
	std::vector<std::string> _positionalArguments;

	bool                                 _isHeadless = true;
	std::optional<std::filesystem::path> _graphPath   = std::nullopt;

	std::optional<std::size_t> _sizeX = std::nullopt;
	std::optional<std::size_t> _sizeY = std::nullopt;
	std::optional<std::size_t> _sizeZ = std::nullopt;

	std::optional<unsigned int> _time = std::nullopt;

	std::optional<Backend> _backend = std::nullopt;

	[[noreturn]]
	void usage(int exit_code);

	void parse(int argc, char** argv);

	void handleOption(char shortopt);

	[[noreturn]]
	static void fail(std::string_view str, std::errc e);

	template<typename T>
	static T fromString(std::string_view str)
	{
		T result{};

		auto [ptr, ec] = std::from_chars(str.begin(), str.begin()+str.size(), result);

		if(ec == std::errc())
			return result;

		fail(str, ec);
		std::unreachable();
	}
};

}
