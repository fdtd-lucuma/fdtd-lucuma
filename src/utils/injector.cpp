// Una GUI para fdtd
// Copyright © 2025-2026 Otreblan
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

module lucuma.utils;

import lucuma.legacy_headers.entt;
import std.compat;

namespace lucuma::utils
{

Injector::~Injector()
{
	for(const auto& f: std::ranges::reverse_view(deleters))
		f();
}

Injector::LinkerWatcher Injector::preLink(entt::type_info type)
{
	return LinkerWatcher(*this, type);
}

Injector::LinkerWatcher::LinkerWatcher(Injector& injector, entt::type_info type):
	injector(injector)
{
	if(injector.dependencies.empty() || injector.dependencies.top() != type)
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

void Injector::printEdges(const std::filesystem::path& path, const std::string_view removePrefix) const
{
	auto ofs = std::ofstream(path);

	if(ofs.is_open())
	{
		StreamEdgeWriter writer{ofs};
		printEdges(writer, removePrefix);
	}
	else
		perror(path.c_str());
}

template<>
entt::registry& Injector::inject<entt::registry>()
{
	return registry;
}

template<>
entt::dispatcher& Injector::inject<entt::dispatcher>()
{
	return dispatcher;
}

}
