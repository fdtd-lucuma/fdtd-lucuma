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

module lucuma.services.basic;

import std;
import lucuma.legacy_headers.xdg_utils_cxx;

namespace lucuma::services::basic
{

PathCommon::PathCommon([[maybe_unused]]Injector& injector):
	settings(injector.inject<Settings>())
{
	init();
}

std::vector<std::filesystem::path> PathCommon::createPath(std::string_view filePreffix) const
{
	return basePath |
		std::views::transform([=](auto&& x){
			return x / filePreffix;
		}) |
		std::ranges::to<std::vector>()
	;
}

void PathCommon::init()
{
	createPath();
}

void PathCommon::createPath()
{
	//TODO: Override by config or something. Or maybe only by envars to avoid
	//dependency loops.
#ifdef NDEBUG
	basePath.emplace_back(XdgUtils::BaseDir::XdgDataHome()) /= PROJECT_NAME;
#endif

	basePath.emplace_back(DATA_DIR);
}

}
