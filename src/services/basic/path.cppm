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

export module lucuma.services.basic:path;

import lucuma.utils;
import std;

namespace lucuma::services::basic
{

using namespace lucuma::utils;

class PathCommon;

class PathBase
{
public:
	PathBase([[maybe_unused]]Injector& injector, std::string_view filePreffix);

	std::span<const std::filesystem::path> getPath() const;

protected:
	PathCommon& common;

	/// Like $PATH
	std::vector<std::filesystem::path> path;

};

template<std::size_t n>
struct FixedString
{
	char str[n];

	constexpr FixedString(const char (&_str)[n])
	{
		std::copy_n(_str, n, str);
	}

	constexpr operator std::string_view() const
	{
		return {str, n-1};
	}

};

struct FileChecks
{
	bool mustExist       = true;
	bool mustBeDirectory = false;
	bool mustBeFile      = false;
};

export template<FixedString filePreffix = {""}, FileChecks checks = {}>
class Path: public PathBase
{
public:
	Path([[maybe_unused]]Injector& injector):
		PathBase(injector, filePreffix)
	{ }

	std::filesystem::path find(const std::filesystem::path& file) const
	{
		std::filesystem::path result(file);

		if(result.is_absolute())
		{
			return result;
		}

		for(const auto& pathDir: path)
		{
			(result = pathDir) /= file;

			if(check(result))
				return result;
		}

		return (result = file);
	}

	std::filesystem::path operator/(const std::filesystem::path& file) const
	{
		return find(file);
	}

private:
	static bool check(std::filesystem::file_status status)
	{
		if constexpr(checks.mustExist)
		{
			if(!std::filesystem::exists(status))
				return false;
		}
		if constexpr(checks.mustBeDirectory)
		{
			if(!std::filesystem::is_directory(status))
				return false;
		}
		if constexpr(checks.mustBeFile)
		{
			if(!std::filesystem::is_regular_file(status))
				return false;
		}

		return true;
	}

	static bool check(const std::filesystem::path& file)
	{
		std::error_code ec;

		auto status = std::filesystem::is_symlink(file) ?
			std::filesystem::symlink_status(file, ec) :
			std::filesystem::status(file, ec)
		;

		if(ec)
			return false;

		return check(status);
	}
};

}
