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
	Field           field           = Field::Electric;
	VectorComponent vectorComponent = VectorComponent::Magnitude;
	Plane           plane           = Plane::XY;
	int             planeIndex      = 0;
	float           multiplier      = 1000;
	int             nThStep         = 1;
};

template <typename T>
void clampPositive(T& x, const T& lo = (T)1)
{
	x = std::clamp(x, lo, std::numeric_limits<T>::max());
}

template <typename T>
T normalizeMinfToInf(T x)
{
	return tanh(x);
}

template <>
_Float16 normalizeMinfToInf(_Float16 x)
{
	return normalizeMinfToInf<float>(x);
}

template <typename T>
T normalize0ToInf(T x)
{
	return x/((T)1+x);
}

template <typename T>
T magnitude(T x, T y, T z, T multiplier)
{
	return normalize0ToInf(multiplier*std::sqrt(x*x+y*y+z*z));
}

template <>
_Float16 magnitude(_Float16 x, _Float16 y, _Float16 z, _Float16 multiplier)
{
	return magnitude<float>(x, y, z, multiplier);
}

template<typename T, typename E, typename L, typename A>
requires (Kokkos::mdspan<T,E,L,A>::rank() == 2)
constexpr bool fastestFromLeft(Kokkos::mdspan<T,E,L,A> plane) {
	return plane.mapping().stride(0) > plane.mapping().stride(1);
}

template <typename T>
class HeatmapData
{
public:
	HeatmapData() = default;

	template <typename T2, typename E, typename L, typename A>
	requires (Kokkos::mdspan<T2,E,L,A>::rank() == 2)
	void fill(Kokkos::mdspan<T2,E,L,A> plane, T multiplier)
	{
		if(plane.empty())
		{
			resize(0, 0);
			return;
		}

		resize(plane.extent(0), plane.extent(1));

		// TODO: Weird layouts

		std::size_t bi = 0;

		if(fastestFromLeft(plane))
		{
			for(std::size_t i = 0; i < sizeX; i++)
			{
				for(std::size_t j = 0; j < sizeY; j++)
				{
					buffer[bi++] = normalizeMinfToInf(multiplier*plane[i,j]);
				}
			}
		}
		else
		{
			for(std::size_t j = 0; j < sizeY; j++)
			{
				for(std::size_t i = 0; i < sizeX; i++)
				{
					buffer[bi++] = normalizeMinfToInf(multiplier*plane[i,j]);
				}
			}
		}
	}

	template <typename T2,
			typename E, typename L, typename A,
			typename E2, typename L2, typename A2,
			typename E3, typename L3, typename A3
		>
	requires (Kokkos::mdspan<T2,E,L,A>::rank() == 2)
	void fill(
			Kokkos::mdspan<T2,E,L,A> xPlane,
			Kokkos::mdspan<T2,E2,L2,A2> yPlane,
			Kokkos::mdspan<T2,E3,L3,A3> zPlane,
			T multiplier
			)
	{
		if(xPlane.empty() || yPlane.empty() || zPlane.empty())
		{
			resize(0, 0);
			return;
		}

		resize(std::min(std::min(xPlane.extent(0), yPlane.extent(0)), zPlane.extent(0)), std::min(std::min(xPlane.extent(1), yPlane.extent(1)), zPlane.extent(1)));

		// TODO: Weird layouts

		std::size_t bi = 0;
		if(fastestFromLeft(xPlane))
		{
			for(std::size_t i = 0; i < sizeX; i++)
			{
				for(std::size_t j = 0; j < sizeY; j++)
				{
					buffer[bi++] = magnitude(xPlane[i,j], yPlane[i,j], zPlane[i,j], multiplier);
				}
			}
		}
		else //TODO: Change heatmap row major column major
		{
			for(std::size_t j = 0; j < sizeY; j++)
			{
				for(std::size_t i = 0; i < sizeX; i++)
				{
					buffer[bi++] = magnitude(xPlane[i,j], yPlane[i,j], zPlane[i,j], multiplier);
				}
			}
		}
	}


	const T* data() const
	{
		return buffer.empty() ? nullptr : buffer.data();
	}

	std::span<const T> getBuffer() const
	{
		return buffer;
	}

	std::size_t getSizeX() const
	{
		return sizeX;
	}

	std::size_t getSizeY() const
	{
		return sizeY;
	}

private:
	std::vector<T> buffer;
	std::size_t sizeX = 0;
	std::size_t sizeY = 0;

	void resize(std::size_t x, std::size_t y)
	{
		sizeX = x;
		sizeY = y;

		buffer.resize(sizeX*sizeY);
	}
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

	HeatmapData<T> heatmapData;

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
		utils::imgui::Combo("Field type", &plotInfo.field);
		utils::imgui::Combo("Field component", &plotInfo.vectorComponent);
		utils::imgui::Combo("Plane", &plotInfo.plane);
		ImGui::SliderInt("Plane index", &plotInfo.planeIndex, 0, getMaxPlaneIndex(), "%d", ImGuiSliderFlags_AlwaysClamp);
		ImGui::InputFloat("Multiplier", &plotInfo.multiplier, 10.f, 100.f);
		ImGui::InputInt("Plot every nTh step", &plotInfo.nThStep);

		clampPositive(plotInfo.nThStep);
		clampPositive(plotInfo.multiplier, 0.f);
	}

	void plotHeatmap()
	{
		if(timeI % plotInfo.nThStep == 0)
			fillHeatmapData();

		constexpr auto colormap  = ImPlotColormap_Jet;
		constexpr auto axisFlags = ImPlotAxisFlags_NoDecorations;

		ImPlot::PushColormap(colormap);

		const auto availSize = ImGui::GetContentRegionAvail();

		if(ImPlot::BeginPlot("##Heatmap", ImVec2(availSize.x, availSize.x)))
		{
			ImPlot::SetupAxes(nullptr, nullptr, axisFlags, axisFlags);
			ImPlot::PlotHeatmap("##heatmap", heatmapData.data(), heatmapData.getSizeX(), heatmapData.getSizeY(), 0, 1, nullptr);

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
	void fillHeatmapData(const data_t<typename PrecisionTraits<precision>::type>& data)
	{
		const auto dim = toDim(plotInfo.vectorComponent);

		if(dim.has_value())
		{
			auto matrix = getMatrix(data, plotInfo.field, dim.value());

			magic_enum::enum_switch([&](auto dim)
			{
				auto plane = slice<dim>(matrix, plotInfo.planeIndex);
				heatmapData.fill(plane, plotInfo.multiplier);
			}, toDim(plotInfo.plane));
		}
		else
		{

			auto xMatrix = getMatrix(data, plotInfo.field, Dim::X);
			auto yMatrix = getMatrix(data, plotInfo.field, Dim::Y);
			auto zMatrix = getMatrix(data, plotInfo.field, Dim::Z);

			magic_enum::enum_switch([&](auto dim)
			{
				auto xPlane = slice<dim>(xMatrix, plotInfo.planeIndex);
				auto yPlane = slice<dim>(yMatrix, plotInfo.planeIndex);
				auto zPlane = slice<dim>(zMatrix, plotInfo.planeIndex);

				heatmapData.fill(xPlane, yPlane, zPlane, plotInfo.multiplier);
			}, toDim(plotInfo.plane));
		}
	}

	//template <>
	//void fillHeatmapData(const vulkan_components::FdtdData<T>& data)
	//{
	//	//TODO
	//}

	int getMaxPlaneIndex()
	{
		return getMaxPlaneIndex(getData());
	}

	template <template<typename> typename data_tt>
	int getMaxPlaneIndex(const data_tt<typename PrecisionTraits<precision>::type>& data)
	{
		const auto dim = toDim(plotInfo.vectorComponent);

		if(dim.has_value())
		{
			return getMaxPlaneIndex(getMatrix(data, plotInfo.field, dim.value()));
		}
		else
		{
			int max = std::numeric_limits<int>::max();

			magic_enum::enum_for_each<Dim>([&, this] (auto dim) {
				max = std::min(max, getMaxPlaneIndex(getMatrix(data, plotInfo.field, dim)));
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
	int getMaxPlaneIndex(Kokkos::mdspan<T2,E,L,A> matrix)
	{
		if(matrix.empty())
			return std::numeric_limits<int>::max();

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
