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

import lucuma.utils;
import std;
import magic_enum;

namespace lucuma::services::basic
{

using namespace utils;

export class ArgumentParser
{
public:
	ArgumentParser(int argc, char** argv);

	std::string_view             argv0() const;
	std::span<const std::string> positionalArguments() const;

	bool isHeadless() const;
	bool debug() const;
	bool tracy() const;
	const std::optional<std::filesystem::path>& graphPath() const;

	std::optional<std::size_t> sizeX() const;
	std::optional<std::size_t> sizeY() const;
	std::optional<std::size_t> sizeZ() const;

	std::optional<std::size_t> gaussX() const;
	std::optional<std::size_t> gaussY() const;
	std::optional<std::size_t> gaussZ() const;

	std::optional<float> deltaT() const;

	std::optional<float> deltaX() const;
	std::optional<float> deltaY() const;
	std::optional<float> deltaZ() const;

	std::optional<float> imp0() const;
	std::optional<float> Cr() const;
	std::optional<float> gaussSigma() const;

	const std::optional<std::filesystem::path>& Hx0() const;
	const std::optional<std::filesystem::path>& Hy0() const;
	const std::optional<std::filesystem::path>& Hz0() const;

	const std::optional<std::filesystem::path>& Chxh0() const;
	const std::optional<std::filesystem::path>& Chyh0() const;
	const std::optional<std::filesystem::path>& Chzh0() const;

	const std::optional<std::filesystem::path>& Chxe0() const;
	const std::optional<std::filesystem::path>& Chye0() const;
	const std::optional<std::filesystem::path>& Chze0() const;

	const std::optional<std::filesystem::path>& CMhx0() const;
	const std::optional<std::filesystem::path>& CMhy0() const;
	const std::optional<std::filesystem::path>& CMhz0() const;

	const std::optional<std::filesystem::path>& mux0() const;
	const std::optional<std::filesystem::path>& muy0() const;
	const std::optional<std::filesystem::path>& muz0() const;

	const std::optional<std::filesystem::path>& muxR0() const;
	const std::optional<std::filesystem::path>& muyR0() const;
	const std::optional<std::filesystem::path>& muzR0() const;

	const std::optional<std::filesystem::path>& Ex0() const;
	const std::optional<std::filesystem::path>& Ey0() const;
	const std::optional<std::filesystem::path>& Ez0() const;

	const std::optional<std::filesystem::path>& Cexe0() const;
	const std::optional<std::filesystem::path>& Ceye0() const;
	const std::optional<std::filesystem::path>& Ceze0() const;

	const std::optional<std::filesystem::path>& Cexh0() const;
	const std::optional<std::filesystem::path>& Ceyh0() const;
	const std::optional<std::filesystem::path>& Cezh0() const;

	const std::optional<std::filesystem::path>& CEEx0() const;
	const std::optional<std::filesystem::path>& CEEy0() const;
	const std::optional<std::filesystem::path>& CEEz0() const;

	const std::optional<std::filesystem::path>& epsx0() const;
	const std::optional<std::filesystem::path>& epsy0() const;
	const std::optional<std::filesystem::path>& epsz0() const;

	const std::optional<std::filesystem::path>& epsxR0() const;
	const std::optional<std::filesystem::path>& epsyR0() const;
	const std::optional<std::filesystem::path>& epszR0() const;

	const std::optional<std::filesystem::path>& eyx00() const;
	const std::optional<std::filesystem::path>& ezx00() const;
	const std::optional<std::filesystem::path>& eyx10() const;
	const std::optional<std::filesystem::path>& ezx10() const;

	const std::optional<std::filesystem::path>& exy00() const;
	const std::optional<std::filesystem::path>& ezy00() const;
	const std::optional<std::filesystem::path>& exy10() const;
	const std::optional<std::filesystem::path>& ezy10() const;

	const std::optional<std::filesystem::path>& exz00() const;
	const std::optional<std::filesystem::path>& eyz00() const;
	const std::optional<std::filesystem::path>& exz10() const;
	const std::optional<std::filesystem::path>& eyz10() const;

	std::optional<unsigned int> time() const;

	std::optional<Backend>   backend()   const;
	std::optional<Precision> precision() const;
	std::optional<SaveAs>    saveAs()    const;

private:
	std::string              _argv0;
	std::vector<std::string> _positionalArguments;

	bool                                 _isHeadless = true;
	bool                                 _debug      = false;
	bool                                 _tracy      = false;
	std::optional<std::filesystem::path> _graphPath   = std::nullopt;

	std::optional<std::size_t> _sizeX = std::nullopt;
	std::optional<std::size_t> _sizeY = std::nullopt;
	std::optional<std::size_t> _sizeZ = std::nullopt;

	std::optional<std::size_t> _gaussX = std::nullopt;
	std::optional<std::size_t> _gaussY = std::nullopt;
	std::optional<std::size_t> _gaussZ = std::nullopt;

	std::optional<float> _deltaT = std::nullopt;

	std::optional<float> _deltaX = std::nullopt;
	std::optional<float> _deltaY = std::nullopt;
	std::optional<float> _deltaZ = std::nullopt;

	std::optional<float> _imp0 = std::nullopt;
	std::optional<float> _Cr = std::nullopt;
	std::optional<float> _gaussSigma = std::nullopt;

	std::optional<std::filesystem::path> _Hx0 = std::nullopt;
	std::optional<std::filesystem::path> _Hy0 = std::nullopt;
	std::optional<std::filesystem::path> _Hz0 = std::nullopt;

	std::optional<std::filesystem::path> _Chxh0 = std::nullopt;
	std::optional<std::filesystem::path> _Chyh0 = std::nullopt;
	std::optional<std::filesystem::path> _Chzh0 = std::nullopt;

	std::optional<std::filesystem::path> _Chxe0 = std::nullopt;
	std::optional<std::filesystem::path> _Chye0 = std::nullopt;
	std::optional<std::filesystem::path> _Chze0 = std::nullopt;

	std::optional<std::filesystem::path> _CMhx0 = std::nullopt;
	std::optional<std::filesystem::path> _CMhy0 = std::nullopt;
	std::optional<std::filesystem::path> _CMhz0 = std::nullopt;

	std::optional<std::filesystem::path> _mux0 = std::nullopt;
	std::optional<std::filesystem::path> _muy0 = std::nullopt;
	std::optional<std::filesystem::path> _muz0 = std::nullopt;

	std::optional<std::filesystem::path> _muxR0 = std::nullopt;
	std::optional<std::filesystem::path> _muyR0 = std::nullopt;
	std::optional<std::filesystem::path> _muzR0 = std::nullopt;

	std::optional<std::filesystem::path> _Ex0 = std::nullopt;
	std::optional<std::filesystem::path> _Ey0 = std::nullopt;
	std::optional<std::filesystem::path> _Ez0 = std::nullopt;

	std::optional<std::filesystem::path> _Cexe0 = std::nullopt;
	std::optional<std::filesystem::path> _Ceye0 = std::nullopt;
	std::optional<std::filesystem::path> _Ceze0 = std::nullopt;

	std::optional<std::filesystem::path> _Cexh0 = std::nullopt;
	std::optional<std::filesystem::path> _Ceyh0 = std::nullopt;
	std::optional<std::filesystem::path> _Cezh0 = std::nullopt;

	std::optional<std::filesystem::path> _CEEx0 = std::nullopt;
	std::optional<std::filesystem::path> _CEEy0 = std::nullopt;
	std::optional<std::filesystem::path> _CEEz0 = std::nullopt;

	std::optional<std::filesystem::path> _epsx0 = std::nullopt;
	std::optional<std::filesystem::path> _epsy0 = std::nullopt;
	std::optional<std::filesystem::path> _epsz0 = std::nullopt;

	std::optional<std::filesystem::path> _epsxR0 = std::nullopt;
	std::optional<std::filesystem::path> _epsyR0 = std::nullopt;
	std::optional<std::filesystem::path> _epszR0 = std::nullopt;

	std::optional<std::filesystem::path> _eyx00 = std::nullopt;
	std::optional<std::filesystem::path> _ezx00 = std::nullopt;
	std::optional<std::filesystem::path> _eyx10 = std::nullopt;
	std::optional<std::filesystem::path> _ezx10 = std::nullopt;

	std::optional<std::filesystem::path> _exy00 = std::nullopt;
	std::optional<std::filesystem::path> _ezy00 = std::nullopt;
	std::optional<std::filesystem::path> _exy10 = std::nullopt;
	std::optional<std::filesystem::path> _ezy10 = std::nullopt;

	std::optional<std::filesystem::path> _exz00 = std::nullopt;
	std::optional<std::filesystem::path> _eyz00 = std::nullopt;
	std::optional<std::filesystem::path> _exz10 = std::nullopt;
	std::optional<std::filesystem::path> _eyz10 = std::nullopt;

	std::optional<unsigned int> _time = std::nullopt;

	std::optional<Backend>   _backend   = std::nullopt;
	std::optional<Precision> _precision = std::nullopt;
	std::optional<SaveAs>    _saveAs    = std::nullopt;

	[[noreturn]]
	void usage(int exit_code);

	void parse(int argc, char** argv);

	void handleOption(int shortopt);

	[[noreturn]]
	static void fail(std::string_view str, std::errc e);

	template<typename T>
	requires std::is_arithmetic_v<T>
	static T fromString(std::string_view str)
	{
		T value{};

		auto [ptr, ec] = std::from_chars(str.begin(), str.begin()+str.size(), value);

		if(ec == std::errc())
			return value;

		fail(str, ec);
		std::unreachable();
	}

	template<typename T>
	requires std::is_enum_v<T>
	static T fromString(std::string_view str)
	{
		auto value = magic_enum::enum_cast<T>(str);

		if(value.has_value())
			return value.value();

		fail(str, std::errc::invalid_argument);
		std::unreachable();
	}

	template<typename T>
	static void fromString(std::optional<T>& result, std::string_view str)
	{
		result.emplace(fromString<T>(str));
	}

	template<typename T>
	static T fromString(std::string_view str)
	{
		return T(str);
	}

};

}
