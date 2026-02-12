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
import lucuma.legacy_headers.nativefiledialog_extended;
import lucuma.components;
import lucuma.services.window;

import imgui;
import glm;
import magic_enum;
import std;

namespace lucuma::systems
{

using namespace lucuma::utils;

SimulationList::SimulationList(Systems& _systems):
	Base(_systems),
	settings(_systems.inject<Settings>()),
	instantiator(_systems.inject<Instantiator>()),
	registry(_systems.inject<entt::registry>())
{
	init();
}

void SimulationList::init()
{
}

void pickFile(std::string_view label, std::filesystem::path& path)
{
	//TODO: Make then button like inputScalar

	constexpr std::array<nfdnfilteritem_t, 1> filters = {
		{{
			.name = "Plain text",
			.spec = "txt",
		}}
	};

	ImGui::BeginGroup();
	ImGui::PushID(label.begin());
	ImGui::InputText("", &path);
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

	if(!path.empty() && !std::filesystem::exists(path)) //TODO: Style
		std::println(std::cerr, "{} does not exists", path.native());

	if(ImGui::Button("Search"))
	{
		NFD::UniquePathN outPath;
		switch(NFD::OpenDialog(outPath, filters.data(), filters.size(), path.parent_path().c_str()))
		{
			case NFD_OKAY:
				path.assign(outPath.get());
				break;
			case NFD_CANCEL:
				std::println(std::cerr, "Cancel");
				break;
			case NFD_ERROR:
				std::println(std::cerr, "Error: {}", NFD::GetError());
				break;
		}
	}

	ImGui::SameLine();
	ImGui::Text("%s", label.begin());

	ImGui::PopID();
	ImGui::EndGroup();

}

void SimulationList::update([[maybe_unused]]const events::Update& event)
{
	if(ImGui::Begin("FDTD"))
	{
		ImGui::Text("%s", "Simulations:");

		simulationTable();

		if(ImGui::Button("New simulation"))
		{
			resetNewSimulationInfo();
			ImGui::OpenPopup("New simulation");
		}

		newSimulationPopup();
	}

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

	if(ImGui::BeginTable("Simulations", 5, flags))
	{
		ImGui::TableSetupColumn("Id");
		ImGui::TableSetupColumn("Actions");
		ImGui::TableSetupColumn("Backend type");
		ImGui::TableSetupColumn("Precision");
		ImGui::TableSetupColumn("Progress");
		ImGui::TableHeadersRow();

		for(auto &&[id, info]: registry.view<components::SimulationInfo>().each())
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

			ImGui::TableNextColumn();
			utils::imgui::ProgressBar(info.timeI, info.maxTime);
		}

		ImGui::EndTable();
	}
}

void SimulationList::rowActions(entt::entity id)
{
	bool isPaused = registry.view<components::Paused>().contains(id);

	ImGui::BeginDisabled(!isPaused);
	if(ImGui::Button("Start"))
	{
		registry.remove<components::Paused>(id);
	}
	ImGui::EndDisabled();

	ImGui::SameLine();

	ImGui::BeginDisabled(isPaused);

	if(ImGui::Button("Stop"))
	{
		registry.emplace<components::Paused>(id);
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
		info.basicDeltaSize[i] = info.basicSize[i]*10/info.size[i];

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

		ImGui::SeparatorText("Starting values");
		newSimulationStartingValues();


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
	if(ImGui::BeginTabBar("Inputs"))
	{
		if(ImGui::BeginTabItem("Human"))
		{
			newSimulationHumanInputs();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Basic"))
		{
			newSimulationBasicInputs();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

}

constexpr const char* cmFormat = "%.2fcm";
constexpr const char* mmFormat = "%.2fmm";
constexpr const char* mFormat = "%.2fm";
constexpr const char* nsFormat = "%.6fns";
constexpr const char* sFormat = "%.2fs";

void SimulationList::newSimulationHumanInputs()
{
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

	newSimulationInfo.type = InputType::HUMAN;
}

void SimulationList::newSimulationBasicInputs()
{
	ImGui::PushItemWidth(400);
	ImGui::InputScalarN("Matrix size", ImGuiDataType_U32, newSimulationInfo.size, 3, &step, &fastStep);
	ImGui::InputScalarN("Source position (in matrix)", ImGuiDataType_U32, newSimulationInfo.gaussPosition, 3, &step, &fastStep);
	ImGui::PopItemWidth();
	ImGui::InputScalar("Time steps", ImGuiDataType_U32, &newSimulationInfo.maxTime, &step, &fastStep);
	ImGui::InputFloat("DeltaT", &newSimulationInfo.deltaT, 0.0f, 0.0f, sFormat);
	ImGui::InputFloat("Delta X", &newSimulationInfo.basicDeltaSize[0], fStep, fFastStep, mFormat);
	ImGui::InputFloat("Delta Y", &newSimulationInfo.basicDeltaSize[1], fStep, fFastStep, mFormat);
	ImGui::InputFloat("Delta Z", &newSimulationInfo.basicDeltaSize[2], fStep, fFastStep, mFormat);
	ImGui::InputFloat("Epsilon", &newSimulationInfo.epsilon, fStep, fFastStep);
	ImGui::InputFloat("Gaussian sigma", &newSimulationInfo.gaussSigma);

	newSimulationInfo.type = InputType::BASIC;
}

void SimulationList::newSimulationBackends()
{
	utils::imgui::Combo<Backend>("Backend type", &newSimulationInfo.backend);
	utils::imgui::Combo<Precision>("Precision", &newSimulationInfo.precision);
}

void SimulationList::newSimulationStartingValues()
{
	constexpr ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DrawLinesFull;

	if(ImGui::TreeNodeEx("Magnetic", flags))
	{
		if(ImGui::TreeNodeEx("Field", flags))
		{
			pickFile("Hx_0", newSimulationInfo.Hx0);
			pickFile("Hy_0", newSimulationInfo.Hy0);
			pickFile("Hz_0", newSimulationInfo.Hz0);

			ImGui::TreePop();
		}

		if(ImGui::TreeNodeEx("Coefficient", flags))
		{
			pickFile("Chxh_0", newSimulationInfo.Chxh0);
			pickFile("Chyh_0", newSimulationInfo.Chyh0);
			pickFile("Chzh_0", newSimulationInfo.Chzh0);

			pickFile("Chxe_0", newSimulationInfo.Chxe0);
			pickFile("Chye_0", newSimulationInfo.Chye0);
			pickFile("Chze_0", newSimulationInfo.Chze0);

			ImGui::TreePop();
		}

		if(ImGui::TreeNodeEx("Conductivity", flags))
		{
			pickFile("CMhx_0", newSimulationInfo.CMhx0);
			pickFile("CMhy_0", newSimulationInfo.CMhy0);
			pickFile("CMhz_0", newSimulationInfo.CMhz0);

			ImGui::TreePop();
		}

		if(ImGui::TreeNodeEx("Permeability", flags))
		{
			pickFile("mux_0", newSimulationInfo.mux0);
			pickFile("muy_0", newSimulationInfo.muy0);
			pickFile("muz_0", newSimulationInfo.muz0);

			pickFile("muxR_0", newSimulationInfo.muxR0);
			pickFile("muyR_0", newSimulationInfo.muyR0);
			pickFile("muzR_0", newSimulationInfo.muzR0);

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	if(ImGui::TreeNodeEx("Electric", flags))
	{
		if(ImGui::TreeNodeEx("Field", flags))
		{
			pickFile("Ex_0", newSimulationInfo.Ex0);
			pickFile("Ey_0", newSimulationInfo.Ey0);
			pickFile("Ez_0", newSimulationInfo.Ez0);

			ImGui::TreePop();
		}

		if(ImGui::TreeNodeEx("Coefficient", flags))
		{
			pickFile("Cexe_0", newSimulationInfo.Cexe0);
			pickFile("Ceye_0", newSimulationInfo.Ceye0);
			pickFile("Ceze_0", newSimulationInfo.Ceze0);

			pickFile("Cexh_0", newSimulationInfo.Cexh0);
			pickFile("Ceyh_0", newSimulationInfo.Ceyh0);
			pickFile("Cezh_0", newSimulationInfo.Cezh0);

			ImGui::TreePop();
		}

		if(ImGui::TreeNodeEx("Conductivity", flags))
		{
			pickFile("CEEx_0", newSimulationInfo.CEEx0);
			pickFile("CEEy_0", newSimulationInfo.CEEy0);
			pickFile("CEEz_0", newSimulationInfo.CEEz0);

			ImGui::TreePop();
		}

		if(ImGui::TreeNodeEx("Permittivity", flags))
		{
			pickFile("epsx_0", newSimulationInfo.epsx0);
			pickFile("epsy_0", newSimulationInfo.epsy0);
			pickFile("epsz_0", newSimulationInfo.epsz0);

			pickFile("epsxR_0", newSimulationInfo.epsxR0);
			pickFile("epsyR_0", newSimulationInfo.epsyR0);
			pickFile("epszR_0", newSimulationInfo.epszR0);

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	if(ImGui::TreeNodeEx("ABC", flags))
	{
		if(ImGui::TreeNodeEx("X", flags))
		{
			pickFile("eyx00", newSimulationInfo.eyx00);
			pickFile("ezx00", newSimulationInfo.ezx00);
			pickFile("eyx10", newSimulationInfo.eyx10);
			pickFile("ezx10", newSimulationInfo.ezx10);

			ImGui::TreePop();
		}

		if(ImGui::TreeNodeEx("Y", flags))
		{
			pickFile("exy00", newSimulationInfo.exy00);
			pickFile("ezy00", newSimulationInfo.ezy00);
			pickFile("exy10", newSimulationInfo.exy10);
			pickFile("ezy10", newSimulationInfo.ezy10);

			ImGui::TreePop();
		}

		if(ImGui::TreeNodeEx("Z", flags))
		{
			pickFile("exz00", newSimulationInfo.exz00);
			pickFile("eyz00", newSimulationInfo.eyz00);
			pickFile("exz10", newSimulationInfo.exz10);
			pickFile("eyz10", newSimulationInfo.eyz10);

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
}

void SimulationList::resetNewSimulationInfo()
{
	const auto size = (glm::vec<3, unsigned int>)settings.size();

	newSimulationInfo = FdtdSimulationInfo{
		.type = InputType::HUMAN,

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

		// Magnetic fields

		.Hx0 = {},
		.Hy0 = {},
		.Hz0 = {},

		.Chxh0 = {},
		.Chyh0 = {},
		.Chzh0 = {},

		.Chxe0 = {},
		.Chye0 = {},
		.Chze0 = {},

		.CMhx0 = {},
		.CMhy0 = {},
		.CMhz0 = {},

		.mux0 = {},
		.muy0 = {},
		.muz0 = {},

		.muxR0 = {},
		.muyR0 = {},
		.muzR0 = {},

		// Electric fields

		.Ex0 = {},
		.Ey0 = {},
		.Ez0 = {},

		.Cexe0 = {},
		.Ceye0 = {},
		.Ceze0 = {},

		.Cexh0 = {},
		.Ceyh0 = {},
		.Cezh0 = {},

		.CEEx0 = {},
		.CEEy0 = {},
		.CEEz0 = {},

		.epsx0 = {},
		.epsy0 = {},
		.epsz0 = {},

		.epsxR0 = {},
		.epsyR0 = {},
		.epszR0 = {},

		// ABC's

		.eyx00 = {},
		.ezx00 = {},
		.eyx10 = {},
		.ezx10 = {},

		.exy00 = {},
		.ezy00 = {},
		.exy10 = {},
		.ezy10 = {},

		.exz00 = {},
		.eyz00 = {},
		.exz10 = {},
		.eyz10 = {},
	};
}



void SimulationList::startSimulation()
{
	components::FdtdDataCreateInfo createInfo {
		.size          = {newSimulationInfo.size[0], newSimulationInfo.size[1], newSimulationInfo.size[2]},
		.gaussPosition = {newSimulationInfo.gaussPosition[0], newSimulationInfo.gaussPosition[1], newSimulationInfo.gaussPosition[2]},
		.deltaT        = newSimulationInfo.deltaT/(newSimulationInfo.type == InputType::HUMAN ? 1000000000 : 1),
		.deltaX        = newSimulationInfo.basicDeltaSize[0]/(newSimulationInfo.type == InputType::HUMAN ? 1000 : 1),
		.deltaY        = newSimulationInfo.basicDeltaSize[1]/(newSimulationInfo.type == InputType::HUMAN ? 1000 : 1),
		.deltaZ        = newSimulationInfo.basicDeltaSize[2]/(newSimulationInfo.type == InputType::HUMAN ? 1000 : 1),
		.imp0          = newSimulationInfo.imp0,
		.Cr            = newSimulationInfo.Cr,
		.maxTime       = newSimulationInfo.maxTime,
		.gaussSigma    = newSimulationInfo.gaussSigma,

		// Magnetic fields

		.Hx0 = newSimulationInfo.Hx0,
		.Hy0 = newSimulationInfo.Hy0,
		.Hz0 = newSimulationInfo.Hz0,

		.Chxh0 = newSimulationInfo.Chxh0,
		.Chyh0 = newSimulationInfo.Chyh0,
		.Chzh0 = newSimulationInfo.Chzh0,

		.Chxe0 = newSimulationInfo.Chxe0,
		.Chye0 = newSimulationInfo.Chye0,
		.Chze0 = newSimulationInfo.Chze0,

		.CMhx0 = newSimulationInfo.CMhx0,
		.CMhy0 = newSimulationInfo.CMhy0,
		.CMhz0 = newSimulationInfo.CMhz0,

		.mux0 = newSimulationInfo.mux0,
		.muy0 = newSimulationInfo.muy0,
		.muz0 = newSimulationInfo.muz0,

		.muxR0 = newSimulationInfo.muxR0,
		.muyR0 = newSimulationInfo.muyR0,
		.muzR0 = newSimulationInfo.muzR0,

		// Electric fields

		.Ex0 = newSimulationInfo.Ex0,
		.Ey0 = newSimulationInfo.Ey0,
		.Ez0 = newSimulationInfo.Ez0,

		.Cexe0 = newSimulationInfo.Cexe0,
		.Ceye0 = newSimulationInfo.Ceye0,
		.Ceze0 = newSimulationInfo.Ceze0,

		.Cexh0 = newSimulationInfo.Cexh0,
		.Ceyh0 = newSimulationInfo.Ceyh0,
		.Cezh0 = newSimulationInfo.Cezh0,

		.CEEx0 = newSimulationInfo.CEEx0,
		.CEEy0 = newSimulationInfo.CEEy0,
		.CEEz0 = newSimulationInfo.CEEz0,

		.epsx0 = newSimulationInfo.epsx0,
		.epsy0 = newSimulationInfo.epsy0,
		.epsz0 = newSimulationInfo.epsz0,

		.epsxR0 = newSimulationInfo.epsxR0,
		.epsyR0 = newSimulationInfo.epsyR0,
		.epszR0 = newSimulationInfo.epszR0,

		// ABC's

		.eyx00 = newSimulationInfo.eyx00,
		.ezx00 = newSimulationInfo.ezx00,
		.eyx10 = newSimulationInfo.eyx10,
		.ezx10 = newSimulationInfo.ezx10,

		.exy00 = newSimulationInfo.exy00,
		.ezy00 = newSimulationInfo.ezy00,
		.exy10 = newSimulationInfo.exy10,
		.ezy10 = newSimulationInfo.ezy10,

		.exz00 = newSimulationInfo.exz00,
		.eyz00 = newSimulationInfo.eyz00,
		.exz10 = newSimulationInfo.exz10,
		.eyz10 = newSimulationInfo.eyz10,
	};

	const auto newId = createEntity();

	registry.emplace<components::SimulationInfo>(newId, components::SimulationInfo({
		.maxTime   = createInfo.maxTime,
		.backend   = newSimulationInfo.backend,
		.precision = newSimulationInfo.precision,
	}));

	registry.emplace<components::SimulationPlotInfo>(newId);

	instantiator.get(newSimulationInfo.backend, newSimulationInfo.precision).init(createInfo, newId);
}

}
