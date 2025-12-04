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
import lucuma.legacy_headers.mdspan;

import :base;

import std.compat;
import imgui;
import glm;
import magic_enum;

namespace lucuma::systems
{

using namespace lucuma::utils;
using namespace services::basic;
using namespace services::backends;

struct PlotInfo
{
	Dim             dimension;
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

	std::vector<T> heatmapData;
	std::size_t sizeX = 0;
	std::size_t sizeY = 0;

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
		ImGui::SliderInt("Plane index", &plotInfo.planeIndex, 0, getMaxPlaneIndex(), "%d", ImGuiSliderFlags_AlwaysClamp);
	}

	void plotHeatmap()
	{
		fillHeatmapData();

		constexpr auto colormap  = ImPlotColormap_Jet;
		constexpr auto axisFlags = ImPlotAxisFlags_NoDecorations;

		ImPlot::PushColormap(colormap);

		if(ImPlot::BeginPlot("##Heatmap", ImVec2(225,225)))
		{
			ImPlot::SetupAxes(nullptr, nullptr, axisFlags, axisFlags);
			ImPlot::PlotHeatmap("heat", heatmapData.data(), (int)sizeX, (int)sizeY);

			ImPlot::EndPlot();
		}

		ImPlot::PopColormap();
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

	void fillHeatmapData()
	{
		fillHeatmapData(getData());
	}

	template <template<typename> typename data_t>
	void fillHeatmapData(const data_t<typename PrecisionTraits<precision>::type>& data);

	template <>
	void fillHeatmapData(const components::FdtdData<T>& data)
	{
		//TODO: Handle Magnitude
		auto matrix = getMatrix(data);

		magic_enum::enum_switch([&](auto dim)
		{
			auto plane = slice<dim>(matrix, plotInfo.planeIndex);
		}, toDim(plotInfo.plane));
	}

	template <>
	void fillHeatmapData(const vulkan_components::FdtdData<T>& data)
	{
		//TODO
	}

	int getMaxPlaneIndex()
	{
		return getMaxPlaneIndex(getData());
	}

	template <template<typename> typename data_tt>
	int getMaxPlaneIndex(const data_tt<typename PrecisionTraits<precision>::type>& data)
	{
		return getMaxPlaneIndex(getMatrix(data));
	}

	template <template<typename> typename data_tt>
	data_tt<typename PrecisionTraits<precision>::type>::cmdspan_3d_t getMatrix(const data_tt<typename PrecisionTraits<precision>::type>& data, Field field, Dim dimension)
	{
		switch(magic_enum::enum_fuse(field, dimension).value())
		{
			case magic_enum::enum_fuse(Field::Electric, Dim::X).value():
				return data.Ex();
			case magic_enum::enum_fuse(Field::Electric, Dim::Y).value():
				return data.Ey();
			case magic_enum::enum_fuse(Field::Electric, Dim::Z).value():
				return data.Ez();
			case magic_enum::enum_fuse(Field::Magnetic, Dim::X).value():
				return data.Hx();
			case magic_enum::enum_fuse(Field::Magnetic, Dim::Y).value():
				return data.Hy();
			case magic_enum::enum_fuse(Field::Magnetic, Dim::Z).value():
				return data.Hz();
		}
	}

	template <template<typename> typename data_tt>
	data_tt<typename PrecisionTraits<precision>::type>::cmdspan_3d_t getMatrix(const data_tt<typename PrecisionTraits<precision>::type>& data)
	{
		return getMatrix(data, plotInfo.field, plotInfo.dimension);
	}

	template <typename T2, typename E, typename L, typename A>
	requires (Kokkos::mdspan<T2,E,L,A>::rank() == 3)
	int getMaxPlaneIndex(Kokkos::mdspan<T2,E,L,A> matrix)
	{
		// TODO: Handle weird layouts
		switch(plotInfo.plane)
		{
			case Plane::XY:
				return matrix.extent(2)-1;
			case Plane::XZ:
				return matrix.extent(1)-1;
			case Plane::YZ:
				return matrix.extent(0)-1;
		}
	}

};

}
