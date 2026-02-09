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

export module lucuma.systems:simulation_stepper;

import lucuma.services.basic;
import lucuma.services.backends;
import lucuma.utils;

import :base;

import std;

namespace lucuma::systems
{

using namespace lucuma::utils;

using namespace services::basic;
using namespace services::backends;

export
class SimulationStepper: public Base<SimulationStepper>
{
public:
	SimulationStepper(Systems& _systems);

	void update(const events::Update& event);

private:
	Settings&       settings;
	Instantiator&   instantiator;
	entt::registry& registry;

	void init();
};

}
