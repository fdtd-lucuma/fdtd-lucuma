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

#include <path_config.hpp>

export module fdtd.services.basic:path;

import :settings;
import fdtd.utils;
import std;

namespace fdtd::services::basic
{

using namespace fdtd::utils;

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

export template<FixedString filePreffix>
class Path
{
public:
	Path([[maybe_unused]]Injector& injector):
		settings(injector.inject<Settings>())
	{
		init();
	}


	std::filesystem::path find(const std::filesystem::path& file) const
	{
		std::filesystem::path result;

		if(file.is_absolute())
		{
			return (result = file);
		}

		for(const auto& pathDir: path)
		{
			(result = pathDir) /= file;

			if(std::filesystem::exists(result))
				return result;
		}

		return file;
	}

	std::filesystem::path operator/(const std::filesystem::path& file) const
	{
		return find(file);
	}

private:
	Settings& settings;

	/// Like $PATH
	std::vector<std::filesystem::path> path;

	void init()
	{
		createPath();
	}

	void createPath()
	{
		path.emplace_back(DATA_DIR) /= (std::string_view)filePreffix;
		// TODO: ~/.local/share
	}

};

}
