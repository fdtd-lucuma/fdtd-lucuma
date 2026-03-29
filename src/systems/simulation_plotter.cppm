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

export module lucuma.systems:simulation_plotter;

import lucuma.services.basic;
import lucuma.services.backends;
import lucuma.events;
import lucuma.utils;
import lucuma.utils.imgui;
import lucuma.components;
//import lucuma.services.backends.vulkan_components;
//import lucuma.legacy_headers.implot3d;
import lucuma.legacy_headers.implot;
import lucuma.legacy_headers.mdspan;

import :base;

import glm;
import imgui;
import magic_enum;
import std;

namespace lucuma::systems
{

using namespace lucuma::utils;
using namespace services::basic;
using namespace services::backends;

template <typename T>
void clampPositive(T& x, const T& lo = (T)1)
{
	x = std::clamp(x, lo, std::numeric_limits<T>::max());
}

export template<template <typename> class _data, Precision precision>
class SimulationPlotter: public Base<SimulationPlotter<_data, precision>>
{
public:
	using T      = PrecisionTraits<precision>::type;
	using data_t = _data<T>;
	using base_t = Base<SimulationPlotter<_data, precision>>;

	SimulationPlotter(Systems& _systems):
		base_t(_systems),
		registry(_systems.inject<entt::registry>()),
		settings(_systems.inject<Settings>())
	{
		init();
	}

	void update([[maybe_unused]] const events::Update& event)
	{
		ZoneNamedN(__zone, "Plotter", settings.tracy());
		for(auto&& [id, simulation_info, plot_info, fdtd_data]: registry.view<components::SimulationInfo, components::SimulationPlotInfo, data_t>().each())
		{
			if(!plot_info.openWindow)
				continue;

			ZoneNamed(__zone2, settings.tracy());
			ZoneNameVF(__zone2, "#%d", id);

			char buffer[128];

			const auto result = std::format_to_n(buffer, std::size(buffer)-1, "Plot #{}", id);
			*result.out = '\0';

			drawProgressBar(id, buffer, simulation_info, plot_info, fdtd_data);
		}
	}

private:
	entt::registry& registry;
	Settings&       settings;

	void init()
	{
	}

	void drawProgressBar(entt::entity id, std::string_view title, const components::SimulationInfo& simulation_info, components::SimulationPlotInfo& plot_info, const data_t& data)
	{
		if(ImGui::Begin(title.data(), &plot_info.openWindow))
		{
			ImGui::SeparatorText("Plotting options");
			plotParameters(plot_info, data);

			ImGui::SeparatorText("Plot");
			// TODO: 3D
			//plot3d();
			plotHeatmap(id, simulation_info, plot_info, data);

			ImGui::SeparatorText("Time steps");
			utils::imgui::ProgressBar(simulation_info.timeI, simulation_info.maxTime);
		}

		ImGui::End();
	}

	void plotParameters(components::SimulationPlotInfo& plot_info, const data_t& data)
	{
		utils::imgui::Combo("Field type", &plot_info.field);
		utils::imgui::Combo("Field component", &plot_info.vectorComponent);
		utils::imgui::Combo("Plane", &plot_info.plane);
		ImGui::SliderInt("Plane index", &plot_info.planeIndex, 0, getMaxPlaneIndex(plot_info, data), "%d", ImGuiSliderFlags_AlwaysClamp);
		ImGui::InputFloat("Multiplier", &plot_info.multiplier, 10.f, 100.f);
		ImGui::InputInt("Plot every nTh step", &plot_info.nThStep);

		clampPositive(plot_info.nThStep);
		clampPositive(plot_info.multiplier, 0.f);
	}

	void plotHeatmap(entt::entity id, const components::SimulationInfo& simulation_info, components::SimulationPlotInfo& plot_info, const data_t& data)
	{
		auto& heatmapData = registry.get_or_emplace<components::HeatmapData<T>>(id);

		if(simulation_info.timeI % plot_info.nThStep == 0)
			fillHeatmapData(heatmapData, plot_info, data);

		constexpr auto colormap  = ImPlotColormap_Jet;
		constexpr auto axisFlags = ImPlotAxisFlags_NoDecorations;

		ImPlot::PushColormap(colormap);

		const auto availSize = ImGui::GetContentRegionAvail();

		if(ImPlot::BeginPlot("##Heatmap", ImVec2(availSize.x, availSize.x)))
		{
			ImPlot::SetupAxes(nullptr, nullptr, axisFlags, axisFlags);
			ImPlot::PlotHeatmap("##heatmap", heatmapData.data(), heatmapData.getSizeX(), heatmapData.getSizeY(), 0, 1, nullptr, ImPlotPoint(0,0), ImPlotPoint(1,1), heatmapData.colMajor() ? ImPlotHeatmapFlags_ColMajor : 0);

			ImPlot::EndPlot();
		}

		ImPlot::PopColormap();
	}

	//void plot3d()
	//{
	//	if(!ImPlot3D::BeginPlot("FDTD"))
	//		return;

	//	glm::vec<3, float> gaussF = gaussPosition;

	//	ImPlot3D::PlotScatter("Source", &gaussF.x, &gaussF.y, &gaussF.z, 1);
	//	ImPlot3D::PlotText("Gaussian source", gaussF.x, gaussF.y, gaussF.z);

	//	//TODO: Plot plane

	//	ImPlot3D::EndPlot();
	//}

	template <template<typename> typename data_t>
	void fillHeatmapData(components::HeatmapData<T>& heatmapData, const components::SimulationPlotInfo& plot_info, const data_t<typename PrecisionTraits<precision>::type>& data)
	{
		const auto dim = toDim(plot_info.vectorComponent);

		if(dim.has_value())
		{
			auto matrix = getMatrix(data, plot_info.field, dim.value());

			magic_enum::enum_switch([&](auto dim)
			{
				auto plane = slice<dim>(matrix, plot_info.planeIndex);

				heatmapData.fill(plane, plot_info.multiplier, settings.debug());
			}, toDim(plot_info.plane));
		}
		else
		{

			auto xMatrix = getMatrix(data, plot_info.field, Dim::X);
			auto yMatrix = getMatrix(data, plot_info.field, Dim::Y);
			auto zMatrix = getMatrix(data, plot_info.field, Dim::Z);

			magic_enum::enum_switch([&](auto dim)
			{
				auto xPlane = slice<dim>(xMatrix, plot_info.planeIndex);
				auto yPlane = slice<dim>(yMatrix, plot_info.planeIndex);
				auto zPlane = slice<dim>(zMatrix, plot_info.planeIndex);

				heatmapData.fill(xPlane, yPlane, zPlane, plot_info.multiplier, settings.debug());
			}, toDim(plot_info.plane));
		}
	}

	//template <>
	//void fillHeatmapData(const vulkan_components::FdtdData<T>& data)
	//{
	//	//TODO
	//}

	template <template<typename> typename data_tt>
	int getMaxPlaneIndex(const components::SimulationPlotInfo& plot_info, const data_tt<typename PrecisionTraits<precision>::type>& data)
	{
		const auto dim = toDim(plot_info.vectorComponent);

		if(dim.has_value())
		{
			return getMaxPlaneIndex(plot_info, getMatrix(data, plot_info.field, dim.value()));
		}
		else
		{
			int max = std::numeric_limits<int>::max();

			magic_enum::enum_for_each<Dim>([&] (auto dim) {
				max = std::min(max, getMaxPlaneIndex(plot_info, getMatrix(data, plot_info.field, dim)));
			});

			return max;
		}
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
		return {};
	}

	template <typename T2, typename E, typename L, typename A>
	requires (Kokkos::mdspan<T2,E,L,A>::rank() == 3)
	int getMaxPlaneIndex(const components::SimulationPlotInfo& plot_info, Kokkos::mdspan<T2,E,L,A> matrix)
	{
		if(matrix.empty())
			return std::numeric_limits<int>::max();

		// TODO: Handle weird layouts
		switch(plot_info.plane)
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
