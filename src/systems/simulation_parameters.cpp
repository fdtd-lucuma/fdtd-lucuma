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
import lucuma.legacy_headers.implot3d;
import lucuma.utils;
import lucuma.utils.imgui;

import std;
import glm;
import imgui;
import glm;
import magic_enum;

namespace lucuma::systems
{

using namespace lucuma::utils;

SimulationParameters::SimulationParameters(Systems& _systems):
	Base(_systems),
	settings(_systems.inject<Settings>()),
	registry(_systems.inject<entt::registry>())
{ 
	init();
}

void SimulationParameters::init()
{
	const auto size = (glm::vec<3, unsigned int>)settings.size();

	fdtdInfo = FdtdInfo{
		.basicSize          = {1, 1, 1},
		.basicGaussPosition = {0.5f, 0.5f, 0.5f},
		.basicTime          = 1.f,
		.basicDeltaSize     = {1.f,1.f,1.f},
		.epsilon = 1.f,

		.size          = {size.x, size.y, size.z},
		.gaussPosition = {size.x/2, size.y/2, size.z/2},
		.deltaT        = 1,
		.imp0          = 377,
		.Cr            = (1.f/std::sqrt(3.f)),
		.maxTime       = settings.time(),
		.gaussSigma    = 10,

		.backend       = Backend::vulkan,
		.precision     = Precision::f32,
	};
}

static const int step     = 1;
static const int fastStep = 100;

static const float fStep     = 0.01f;
static const float fFastStep = fStep*100;

void SimulationParameters::update(const events::Update&)
{
	ImGui::Begin("FDTD");

	ImGui::SeparatorText("Simulation parameters");

	basicTab();

	ImGui::SeparatorText("Backends");

	utils::imgui::Combo<Backend>("Backend type", &fdtdInfo.backend);
	utils::imgui::Combo<Precision>("Precision", &fdtdInfo.precision);

	if(ImGui::Button("Start") && !registry.valid(simulationId))
	{
		magic_enum::enum_switch([&](auto precision)
		{
			magic_enum::enum_switch([&](auto backend)
			{
				simulationId = systems.start<Simulation<backend, precision>>();

			}, fdtdInfo.backend);
		}, fdtdInfo.precision);
	}

	ImGui::End();

	// This is broken
	//ImGui::End();

	//ImGui::Begin("Service graph");

	//ImguiGraphnodeEdgeWriter writer;
	//_injector.printEdges(writer);

	//ImGui::End();

	ImPlot3D::ShowDemoWindow();
}

template <std::size_t N>
void clampFPositive(
	float (&v)[N],
	const float (&min)[N] = {0.f, 0.f, 0.f},
	const float (&max)[N] = {
		std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max()
		}
	)
{
	for(std::size_t i = 0; i < N; i++)
	{
		v[i] = std::clamp(v[i], min[i], max[i]);
	}
}

// Return max deltaT in m/s
float maxDeltaTime(float deltaX, float deltaY, float deltaZ, float c)
{
	return 1.f/(c*std::sqrt((1/(deltaX*deltaX))+(1/(deltaY*deltaY))+(1/(deltaZ*deltaZ))));
}

void updateInputs(FdtdInfo& info)
{
	for(std::size_t i = 0; i < 3; i++)
		info.basicDeltaSize[i] = info.basicSize[i]/info.size[i];

	info.maxTime = info.basicTime/info.deltaT;
	info.deltaT = std::min(info.deltaT, maxDeltaTime(info.basicDeltaSize[0]/1000, info.basicDeltaSize[1]/1000, info.basicDeltaSize[2]/1000, 3.f*std::pow(10.f, 8.f))*1000000000);
}

void SimulationParameters::basicTab()
{
	constexpr const char* cmFormat = "%.2fcm";
	constexpr const char* mmFormat = "%.2fmm";
	constexpr const char* nsFormat = "%.6fns";

	if(ImGui::InputScalarN("Size", ImGuiDataType_Float, fdtdInfo.basicSize, 3, &fStep, &fFastStep, cmFormat))
		clampFPositive(fdtdInfo.basicSize);

	ImGui::InputScalarN("Matrix size", ImGuiDataType_U32, fdtdInfo.size, 3, &step, &fastStep);

	if(ImGui::InputScalarN("Source position", ImGuiDataType_Float, fdtdInfo.basicGaussPosition, 3, &fStep, &fFastStep, cmFormat))
		clampFPositive(fdtdInfo.basicGaussPosition, {0.f, 0.f, 0.f}, fdtdInfo.basicSize);

	ImGui::InputFloat("Time", &fdtdInfo.basicTime, fStep, fFastStep, nsFormat);
	ImGui::BeginDisabled();
	ImGui::InputFloat("Delta X", &fdtdInfo.basicDeltaSize[0], fStep, fFastStep, mmFormat); // TODO: Send everything as meters
	ImGui::InputFloat("Delta Y", &fdtdInfo.basicDeltaSize[1], fStep, fFastStep, mmFormat);
	ImGui::InputFloat("Delta Z", &fdtdInfo.basicDeltaSize[2], fStep, fFastStep, mmFormat);
	ImGui::EndDisabled();

	ImGui::InputFloat("DeltaT", &fdtdInfo.deltaT, 0.0f, 0.0f, nsFormat);
	ImGui::InputFloat("Epsilon", &fdtdInfo.epsilon, fStep, fFastStep);

	ImGui::BeginDisabled();
	ImGui::InputScalar("Time steps", ImGuiDataType_U32, &fdtdInfo.maxTime, &step, &fastStep);
	ImGui::EndDisabled();

	ImGui::InputFloat("Gaussian sigma", &fdtdInfo.gaussSigma);

	updateInputs(fdtdInfo);
}


}
