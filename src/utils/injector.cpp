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

constexpr const std::string_view unPreffix(const std::string_view str, const std::string_view prefix)
{
	return std::string_view(std::mismatch(str.begin(), str.end(), prefix.begin()).first, str.end());
}

void Injector::printEdges(std::ostream& os, const std::string_view removePrefix) const
{
	std::print(os,
		"strict digraph \"{0}\" {{\n\tlabel=\"{0} graph\"\n",
		entt::type_id<decltype(*this)>().name()
	);

	for(auto&& [l, r]: dependenciesEdges)
	{
		std::println(os,
			"\t\"{}\" -> \"{}\";",
			unPreffix(l.name(), removePrefix),
			unPreffix(r.name(), removePrefix)
		);
	}

	std::println(os, "{}", "}");
}

void Injector::printEdges(const std::filesystem::path& path, const std::string_view removePrefix) const
{
	auto ofs = std::ofstream(path);

	if(ofs.is_open())
		printEdges(ofs, removePrefix);
	else
		perror(path.c_str());
}

}
