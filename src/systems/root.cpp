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

module lucuma.systems;
import lucuma.utils;
import lucuma.services.backends;
import lucuma.services.window;
import lucuma.utils.imgui;
import lucuma.components;
import lucuma.services.backends.vulkan_components;

import std;
import magic_enum;

import :simulation_plotter;

namespace lucuma::systems
{

using namespace lucuma::utils;

Root::Root(Systems& _systems):
	Base(_systems),
	settings(_systems.inject<Settings>()),
	registry(_systems.inject<entt::registry>())
{
	init();
}

void Root::init()
{
	systems.inject<lucuma::services::window::Filedialog>();

	systems.start<systems::SimulationList>();
	systems.start<systems::SimulationStepper>();

	magic_enum::enum_for_each<Precision>([&](auto precision)
	{
		systems.start<systems::SimulationPlotter<lucuma::components::FdtdData, precision>>();
		systems.start<systems::SimulationPlotter<lucuma::services::backends::vulkan_components::FdtdData, precision>>();

		registerComponentEditor<components::GaussianSource<typename PrecisionTraits<precision>::type>>();
	});

	registerComponentEditor<components::Transform>();
}


}
