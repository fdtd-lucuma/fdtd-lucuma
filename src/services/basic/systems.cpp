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

module lucuma.services.basic;

import std;

namespace lucuma::services::basic
{

Systems::Systems([[maybe_unused]]Injector& _injector):
	dispatcher(_injector.inject<entt::dispatcher>()),
	registry(_injector.inject<entt::registry>()),
	injector(_injector)
{ }

void Systems::stop(entt::entity e)
{
	if(!registry.valid(e))
		return;

	registry.emplace<toStop>(e);
}

void Systems::cleanStopped()
{
	for(auto&& [id, on]: registry.group<toStop>(entt::get<OnSystemEnd>).each())
	{
		on.f(registry, id);
	}

	auto view = registry.view<toStop>();

	registry.destroy(view.begin(), view.end());
}

entt::entity Systems::createEntity()
{
	return createMine<mine>();
}

Systems::~Systems()
{
	stopMine<mine>();

	cleanStopped();
}

}
