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

#include <tracy/Tracy.hpp>

module lucuma.systems;
import lucuma.utils;
import lucuma.utils.imgui;
import lucuma.components;

import imgui;
import magic_enum;
import std;

namespace lucuma::systems
{

using namespace lucuma::utils;

SimulationStepper::SimulationStepper(Systems& _systems):
	Base(_systems),
	settings(_systems.inject<Settings>()),
	instantiator(_systems.inject<Instantiator>()),
	registry(_systems.inject<entt::registry>())
{
	init();
}

void SimulationStepper::init()
{
}

void SimulationStepper::update([[maybe_unused]]const events::Update& event)
{
	ZoneNamedN(__zone, "Stepper", settings.tracy());

	// TODO: Find a way to decouple this from the frame rate
	for(auto&& [id, info]: registry.view<components::SimulationInfo>(entt::exclude<components::Paused>).each())
	{
		ZoneNamed(__zone2, settings.tracy());
		ZoneNameVF(__zone2, "#%d", id);

		if(!instantiator.get(info.backend, info.precision).step(id))
		{
			registry.emplace<components::Paused>(id);
			continue;
		}

		info.timeI++;
	}
}


}
