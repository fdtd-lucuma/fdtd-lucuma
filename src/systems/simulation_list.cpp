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

module lucuma.systems;
import lucuma.utils;
import lucuma.services.backends;
import lucuma.utils.imgui;

import imgui;
import magic_enum;
import std;

namespace lucuma::systems
{

using namespace lucuma::utils;

SimulationList::SimulationList(Systems& _systems):
	Base(_systems),
	settings(_systems.inject<Settings>()),
	registry(_systems.inject<entt::registry>())
{
	init();
}

void SimulationList::init()
{
}

void SimulationList::update([[maybe_unused]]const events::Update& event)
{
	ImGui::Begin("FDTD");

	ImGui::Text("%s", "Simulations:");

	simulationTable();

	if(ImGui::Button("New simulation"))
	{
		resetNewSimulationInfo();
		ImGui::OpenPopup("New simulation");
	}

	newSimulationPopup();

	ImGui::End();
}

void SimulationList::simulationTable()
{
	constexpr ImGuiTableFlags flags =
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_HighlightHoveredColumn
	;

	if(ImGui::BeginTable("Simulations", 4, flags))
	{
		ImGui::TableSetupColumn("Id");
		ImGui::TableSetupColumn("Actions");
		ImGui::TableSetupColumn("Backend type");
		ImGui::TableSetupColumn("Precision");
		ImGui::TableHeadersRow();

		for(auto &&[id, info]: registry.view<FdtdSimulationInfo>().each())
		{
			utils::imgui::EntityCtx ctx(id);

			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			ImGui::Text("%d", id);

			ImGui::TableNextColumn();
			rowActions(id);

			ImGui::TableNextColumn();
			utils::imgui::Enum(info.backend);

			ImGui::TableNextColumn();
			utils::imgui::Enum(info.precision);
		}

		ImGui::EndTable();
	}
}

void SimulationList::rowActions(entt::entity id)
{
	ImGui::BeginDisabled(true);
	if(ImGui::Button("Start"))
	{
		//TODO
	}
	ImGui::EndDisabled();

	ImGui::SameLine();

	ImGui::BeginDisabled(false);

	if(ImGui::Button("Stop"))
	{
		//TODO
	}

	ImGui::EndDisabled();

	ImGui::SameLine();

	ImGui::BeginDisabled(false);
	if(ImGui::Button("Cancel"))
	{
		systems.stop(id);
	}
	ImGui::EndDisabled();
}

template <typename T, std::size_t N>
void clamp(
	T (&v)[N],
	const T (&min)[N] = {(T)0, (T)0, (T)0},
	const T (&max)[N] = {
		std::numeric_limits<T>::max(),
		std::numeric_limits<T>::max(),
		std::numeric_limits<T>::max()
		}
	)
{
	for(std::size_t i = 0; i < N; i++)
	{
		v[i] = std::clamp(v[i], min[i], max[i]);
	}
}

template <std::size_t N>
void calculatePosition(
	unsigned int (&position)[N],
	const float (&basicPosition)[N],
	const float (&basicSize)[N],
	const unsigned int (&size)[N]
	)
{
	for(std::size_t i = 0; i < N; i++)
	{
		position[i] = (basicPosition[i]/basicSize[i])*size[i];
	}
}

// Return max deltaT in m/s
float maxDeltaTime(float deltaX, float deltaY, float deltaZ, float c)
{
	return 1.f/(c*std::sqrt((1/(deltaX*deltaX))+(1/(deltaY*deltaY))+(1/(deltaZ*deltaZ))));
}

void updateInputs(FdtdSimulationInfo& info)
{
	for(std::size_t i = 0; i < 3; i++)
		info.basicDeltaSize[i] = info.basicSize[i]/info.size[i];

	info.maxTime = info.basicTime/info.deltaT;
	info.deltaT = std::min(info.deltaT, maxDeltaTime(info.basicDeltaSize[0]/1000, info.basicDeltaSize[1]/1000, info.basicDeltaSize[2]/1000, 3.f*std::pow(10.f, 8.f))*1000000000);

	calculatePosition(info.gaussPosition, info.basicGaussPosition, info.basicSize, info.size);
}

void SimulationList::clampSizes()
{
	clamp(newSimulationInfo.basicSize);
	clamp(newSimulationInfo.basicGaussPosition, {0.f, 0.f, 0.f}, newSimulationInfo.basicSize);
	clamp(newSimulationInfo.gaussPosition, {0u, 0u, 0u}, newSimulationInfo.size);
}

void SimulationList::newSimulationPopup()
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if(ImGui::BeginPopupModal("New simulation", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SeparatorText("Simulation parameters");
		newSimulationInputs();

		ImGui::SeparatorText("Backends");
		newSimulationBackends();

		if(ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if(ImGui::Button("OK"))
		{
			startSimulation();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

static const int step     = 1;
static const int fastStep = 100;

static const float fStep     = 0.01f;
static const float fFastStep = fStep*100;

void SimulationList::newSimulationInputs()
{
	constexpr const char* cmFormat = "%.2fcm";
	constexpr const char* mmFormat = "%.2fmm";
	constexpr const char* nsFormat = "%.6fns";

	ImGui::PushItemWidth(400);
	if(ImGui::InputScalarN("Size", ImGuiDataType_Float, newSimulationInfo.basicSize, 3, &fStep, &fFastStep, cmFormat))
		clampSizes();

	if(ImGui::InputScalarN("Matrix size", ImGuiDataType_U32, newSimulationInfo.size, 3, &step, &fastStep))
		clampSizes();

	if(ImGui::InputScalarN("Source position", ImGuiDataType_Float, newSimulationInfo.basicGaussPosition, 3, &fStep, &fFastStep, cmFormat))
		clampSizes();

	ImGui::BeginDisabled();
	ImGui::InputScalarN("Source position (in matrix)", ImGuiDataType_U32, newSimulationInfo.gaussPosition, 3, &step, &fastStep);
	ImGui::EndDisabled();

	ImGui::PopItemWidth();

	ImGui::InputFloat("Time", &newSimulationInfo.basicTime, fStep, fFastStep, nsFormat);
	ImGui::BeginDisabled();
	ImGui::InputScalar("Time steps", ImGuiDataType_U32, &newSimulationInfo.maxTime, &step, &fastStep);
	ImGui::EndDisabled();
	ImGui::InputFloat("DeltaT", &newSimulationInfo.deltaT, 0.0f, 0.0f, nsFormat);
	ImGui::BeginDisabled();
	ImGui::InputFloat("Delta X", &newSimulationInfo.basicDeltaSize[0], fStep, fFastStep, mmFormat); // TODO: Send everything as meters
	ImGui::InputFloat("Delta Y", &newSimulationInfo.basicDeltaSize[1], fStep, fFastStep, mmFormat);
	ImGui::InputFloat("Delta Z", &newSimulationInfo.basicDeltaSize[2], fStep, fFastStep, mmFormat);
	ImGui::EndDisabled();

	ImGui::InputFloat("Epsilon", &newSimulationInfo.epsilon, fStep, fFastStep);


	ImGui::InputFloat("Gaussian sigma", &newSimulationInfo.gaussSigma);

	updateInputs(newSimulationInfo);
}

void SimulationList::newSimulationBackends()
{
	utils::imgui::Combo<Backend>("Backend type", &newSimulationInfo.backend);
	utils::imgui::Combo<Precision>("Precision", &newSimulationInfo.precision);
}

void SimulationList::resetNewSimulationInfo()
{
	const auto size = (glm::vec<3, unsigned int>)settings.size();

	newSimulationInfo = FdtdSimulationInfo{
		.basicSize          = {1, 1, 1},
		.basicGaussPosition = {0.5f, 0.5f, 0.5f},
		.basicTime          = 1.f,
		.basicDeltaSize     = {1.f,1.f,1.f},
		.epsilon            = 1.f,

		.size          = {size.x, size.y, size.z},
		.gaussPosition = {size.x/2, size.y/2, size.z/2},
		.deltaT        = 1,
		.imp0          = 377,
		.Cr            = (1.f/std::sqrt(3.f)),
		.maxTime       = settings.time(),
		.gaussSigma    = 10,

		.backend   = Backend::vulkan,
		.precision = Precision::f32,
	};
}

void SimulationList::startSimulation()
{
	components::FdtdDataCreateInfo createInfo {
		.size          = {newSimulationInfo.size[0], newSimulationInfo.size[1], newSimulationInfo.size[2]},
		.gaussPosition = {newSimulationInfo.gaussPosition[0], newSimulationInfo.gaussPosition[1], newSimulationInfo.gaussPosition[2]},
		.deltaT        = newSimulationInfo.deltaT/1000000000,
		.deltaX        = newSimulationInfo.basicDeltaSize[0]/1000,
		.deltaY        = newSimulationInfo.basicDeltaSize[1]/1000,
		.deltaZ        = newSimulationInfo.basicDeltaSize[2]/1000,
		.imp0          = newSimulationInfo.imp0,
		.Cr            = newSimulationInfo.Cr,
		.maxTime       = newSimulationInfo.maxTime,
		.gaussSigma    = newSimulationInfo.gaussSigma,
	};

	const auto newId = registry.create();

	registry.emplace<FdtdSimulationInfo>(newId, newSimulationInfo);

	magic_enum::enum_switch([&](auto precision)
	{
		magic_enum::enum_switch([&](auto backend)
		{
			// TODO: No system, but entity
			systems.start<Simulation<backend, precision>>(createInfo, newId);

		}, newSimulationInfo.backend);
	}, newSimulationInfo.precision);
}

}
