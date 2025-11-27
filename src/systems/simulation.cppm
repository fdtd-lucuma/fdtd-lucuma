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

export module lucuma.systems:simulation;

import lucuma.services.basic;
import lucuma.services.backends;
import lucuma.events;
import lucuma.utils;

import :base;

import std;
import imgui;

namespace lucuma::systems
{

using namespace lucuma::utils;
using namespace services::basic;
using namespace services::backends;

export template<Backend backend, Precision precision>
class Simulation: public Base<Simulation<backend, precision>>
{
public:
	using base_t = Base<Simulation<backend, precision>>;

	Simulation(Systems& _systems):
		base_t(_systems),
		iBackend(_systems.inject<Instantiator>().get(backend, precision)),
		registry(_systems.inject<entt::registry>())
	{
		std::println("Create {} {} simulation", backend, precision);

		// TODO: Input from here
		simulationId = iBackend.init();
	}

	void update([[maybe_unused]] const events::Update& event)
	{
		// TODO: Async step
		if(!iBackend.step(simulationId))
		{
			base_t::selfStop();
			return;
		}

		iBackend.saveFiles(simulationId);

		drawProgressBar();
	}

	virtual ~Simulation()
	{
		std::println("Destroy {} {} simulation", backend, precision);

		//TODO: Find a way to destroy in a destroyer
		base_t::systems.stop(simulationId);
	}

private:
	IBackend&       iBackend;
	entt::registry& registry;

	entt::entity simulationId = entt::null;

	void drawProgressBar()
	{
		ImGui::Begin("Progress");

		ImGui::ProgressBar(-1.f*(float)ImGui::GetTime());

		ImGui::End();
	}

};

}
