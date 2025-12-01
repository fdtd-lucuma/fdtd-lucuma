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
import lucuma.components;
import lucuma.legacy_headers.implot3d;

import :base;

import std.compat;
import imgui;
import glm;

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

	Simulation(Systems& _systems, const components::FdtdDataCreateInfo& createInfo):
		base_t(_systems),
		iBackend(_systems.inject<Instantiator>().get(backend, precision)),
		registry(_systems.inject<entt::registry>()),
		gaussPosition(createInfo.gaussPosition),
		maxTime(createInfo.maxTime)
	{
		std::println("Create {} {} simulation", backend, precision);

		// TODO: Input from here
		simulationId = iBackend.init(createInfo);
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

	svec3 gaussPosition;
	const unsigned int maxTime;
	unsigned int timeI = 0;

	void drawProgressBar()
	{
		ImGui::Begin("Progress");

		float progress = (float)timeI++/maxTime;
		char buffer[32];
		snprintf(buffer, sizeof(buffer)/sizeof(*buffer), "%d/%d", (int)(progress*maxTime), maxTime);

		ImGui::ProgressBar(progress, ImVec2(-std::numeric_limits<float>::min(),0), buffer);

		plot3d();

		ImGui::End();
	}

	void plot3d()
	{
		if(!ImPlot3D::BeginPlot("FDTD"))
			return;

		glm::vec<3, float> gaussF = gaussPosition;

		ImPlot3D::PlotScatter("Source", &gaussF.x, &gaussF.y, &gaussF.z, 1);
		ImPlot3D::PlotText("Gaussian source", gaussF.x, gaussF.y, gaussF.z);

		//TODO: Plot plane

		ImPlot3D::EndPlot();
	}

};

}
