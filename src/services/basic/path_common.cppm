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

#include <path_config.hpp>

export module lucuma.services.basic:path_common;

import lucuma.utils;

import std;
import :settings;

namespace lucuma::services::basic
{

using namespace lucuma::utils;

export class PathCommon
{
public:
	PathCommon(Injector& injector);

	std::vector<std::filesystem::path> createPath(std::string_view filePreffix) const;

private:
	Settings& settings;

	std::vector<std::filesystem::path> basePath;
	void init();
	void createPath();
};

}
