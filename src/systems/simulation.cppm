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
import lucuma.utils.imgui;
import lucuma.components;
import lucuma.services.backends.vulkan_components;
import lucuma.legacy_headers.implot3d;
import lucuma.legacy_headers.implot;

import :base;

import std.compat;
import imgui;
import glm;

namespace lucuma::systems
{

using namespace lucuma::utils;
using namespace services::basic;
using namespace services::backends;

enum class Dimension
{
	X,
	Y,
	Z,
};

enum class Plane
{
	XY,
	XZ,
	YX,
	YZ,
	ZX,
	ZY,
};

enum class Field
{
	Electric,
	Magnetic,
};

enum class VectorComponent
{
	Magnitude,
	X,
	Y,
	Z,
};

struct PlotInfo
{
	Dimension       dimension;
	Field           field;
	VectorComponent vectorComponent;
	Plane           plane;
	int             planeIndex;
};

export template<Backend backend, Precision precision>
class Simulation: public Base<Simulation<backend, precision>>
{
public:
	using T = PrecisionTraits<precision>::type;
	using base_t = Base<Simulation<backend, precision>>;
	using backend_t = BackendTraits<backend>::template type<precision>;
	using data_t = backend_t::data_t;

	Simulation(Systems& _systems, const components::FdtdDataCreateInfo& createInfo):
		base_t(_systems),
		backendService(_systems.inject<backend_t>()),
		registry(_systems.inject<entt::registry>()),
		gaussPosition(createInfo.gaussPosition),
		maxTime(createInfo.maxTime)
	{
		std::println("Create {} {} simulation", backend, precision);

		// TODO: Input from here
		simulationId = backendService.init(createInfo);
	}

	void update([[maybe_unused]] const events::Update& event)
	{
		// TODO: Async step
		if(!backendService.step(simulationId))
		{
			base_t::selfStop();
			return;
		}

		backendService.saveFiles(simulationId);

		drawProgressBar();
	}

	virtual ~Simulation()
	{
		std::println("Destroy {} {} simulation", backend, precision);

		//TODO: Find a way to destroy in a destroyer
		base_t::systems.stop(simulationId);
	}

private:
	backend_t&      backendService;
	entt::registry& registry;

	entt::entity simulationId = entt::null;

	PlotInfo plotInfo;

	svec3 gaussPosition;
	const unsigned int maxTime;
	unsigned int timeI = 0;

	void drawProgressBar()
	{
		ImGui::Begin("Progress");

		float progress = (float)timeI++/maxTime;
		char buffer[32];
		snprintf(buffer, sizeof(buffer)/sizeof(*buffer), "%d/%d", (int)(progress*maxTime), maxTime);

		ImGui::SeparatorText("Plotting options");
		plotParameters();

		ImGui::SeparatorText("Plot");
		// TODO: 3D
		//plot3d();
		plotHeatmap();

		ImGui::SeparatorText("Time steps");
		ImGui::ProgressBar(progress, ImVec2(-std::numeric_limits<float>::min(),0), buffer);

		ImGui::End();
	}

	void plotParameters()
	{
		utils::imgui::Combo("Dimension", &plotInfo.dimension);
		utils::imgui::Combo("Field type", &plotInfo.field);
		utils::imgui::Combo("Field component", &plotInfo.vectorComponent);
		utils::imgui::Combo("Plane", &plotInfo.plane);
		ImGui::SliderInt("Plane index", &plotInfo.planeIndex, 0, getMaxPlaneIndex());
	}

	void plotHeatmap()
	{
		fillHeatmap();
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

	const data_t& getData()
	{
		return registry.get<data_t>(simulationId);
	}

	void fillHeatmap()
	{
		fillHeatmap(getData());
	}

	template <template<typename> typename data_t>
	void fillHeatmap(const data_t<typename PrecisionTraits<precision>::type>& data);

	template <>
	void fillHeatmap(const components::FdtdData<T>& data)
	{
	}

	template <>
	void fillHeatmap(const vulkan_components::FdtdData<T>& data)
	{
	}

	int getMaxPlaneIndex()
	{
		return getMaxPlaneIndex(getData());
	}

	template <template<typename> typename data_tt>
	int getMaxPlaneIndex(const data_tt<typename PrecisionTraits<precision>::type>& data);

	template <>
	int getMaxPlaneIndex(const components::FdtdData<T>& data)
	{
		return 10;
	}

	template <>
	int getMaxPlaneIndex(const vulkan_components::FdtdData<T>& data)
	{
		return 10;
	}
};

}
