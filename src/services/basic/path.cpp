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

module fdtd.services.basic;

import std;

namespace fdtd::services::basic
{

Path::Path([[maybe_unused]]Injector& injector):
	settings(injector.inject<Settings>())
{
	init();
}

void Path::init()
{
	createPath();
}

void Path::createPath()
{
	path.emplace_back(DATA_DIR);
	// TODO: ~/.local/share
}

std::filesystem::path Path::find(const std::filesystem::path& file) const
{
	std::filesystem::path result;

	for(const auto& pathDir: path)
	{
		result = pathDir / file;

		if(std::filesystem::exists(result))
			return result;
	}

	return result;
}

}
