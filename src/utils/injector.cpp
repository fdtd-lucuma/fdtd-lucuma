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

#include <ranges>

module fdtd.utils;

import fdtd.legacy_headers.entt;
import std;

namespace fdtd::utils
{

Injector::~Injector()
{
	for(const auto& f: deleters | std::views::reverse)
		f();
}

Injector::LinkerWatcher Injector::preLink(entt::type_info type)
{
	return LinkerWatcher(*this, type);
}

Injector::LinkerWatcher::LinkerWatcher(Injector& injector, entt::type_info type):
	injector(injector)
{
	if(injector.dependencies.empty() || injector.dependencies.top().hash() != type.hash())
	{
		injector.dependencies.push(type);
		linked = true;
	}
}

Injector::LinkerWatcher::~LinkerWatcher()
{
	if(!linked)
		return;

	entt::type_info top = injector.dependencies.top();
	injector.dependencies.pop();

	if(injector.dependencies.empty())
		return;

	entt::type_info oldtop = injector.dependencies.top();

	injector.dependenciesEdges.emplace_back(oldtop, top);
}

void Injector::printEdges(std::ostream& os) const
{
	os << "digraph Injector {\n";

	for(auto&& [l, r]: dependenciesEdges)
	{
		os
			<< "\t\"" << l.name()
			<< "\" -> \""
			<< r.name() << "\";\n"
		;
	}

	os << "}\n";
}

void Injector::printEdges(const std::filesystem::path& path) const
{
	auto ofs = std::ofstream(path);

	if(ofs.is_open())
		printEdges(ofs);
	else
		perror(path.c_str());
}

}
