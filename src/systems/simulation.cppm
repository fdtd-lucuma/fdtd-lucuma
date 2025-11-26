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
import lucuma.events;
import lucuma.utils;

import :base;

import std;
import imgui;

namespace lucuma::systems
{

using namespace lucuma::utils;
using namespace services::basic;

export template<Backend backend, Precision precision>
class Simulation: public Base<Simulation<backend, precision>>
{
public:
	using base_t = Base<Simulation<backend, precision>>;

	Simulation(Systems& _systems):
		base_t(_systems)
	{
		std::println("Create {} {} simulation", backend, precision);
	}

	void update(const events::Update& event)
	{
		progress = std::min(1.0f, progress+event.deltaTime/1000);

		drawProgressBar();

		if(progress >= 1.0f)
			base_t::selfStop();
	}

private:
	float progress = 0.0f;

	void drawProgressBar()
	{
		ImGui::Begin("Progress");

		ImGui::ProgressBar(progress);

		ImGui::End();
	}

};

}
