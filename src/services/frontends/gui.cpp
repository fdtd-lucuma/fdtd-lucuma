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

module lucuma.services.frontends;

import lucuma.utils;
import lucuma.utils.imgui;
import lucuma.services.window;
import lucuma.services.vulkan;
import lucuma.legacy_headers.entt;
import lucuma.legacy_headers.imgui_graphnode;
import lucuma.legacy_headers.implot3d;
import lucuma.events;

import imgui;
import glm;

import std;

namespace lucuma::services::frontends
{

Gui::Gui([[maybe_unused]]Injector& injector):
	_injector(injector),
	dispatcher(injector.inject<entt::dispatcher>()),
	registry(injector.inject<entt::registry>()),
	glfw(injector.inject<window::Glfw>()),
	graphics(injector.inject<vulkan::Graphics>()),
	triangleDemo(injector.inject<vulkan::TriangleDemo>()),
	imgui(injector.inject<vulkan::Imgui>()),
	settings(injector.inject<basic::Settings>())
{
	init();
}

void Gui::init()
{
	// TODO: Wrap this in a raii helper
	dispatcher.sink<events::Update>().connect<&Gui::update>(*this);

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

void Gui::start()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	auto lastTime = currentTime;

	float timeDelta = 1.f/60;

	while(!glfw.shouldClose())
	{
		lastTime = currentTime;

		glfw.pollEvents();

		drawFrame(timeDelta);

		currentTime = std::chrono::high_resolution_clock::now();
		timeDelta   = std::chrono::duration<float, std::chrono::seconds::period>(currentTime-lastTime).count();
	}
}

void Gui::drawFrame(float timeDelta)
{
	dispatcher.trigger(events::FrameStart{timeDelta});
	dispatcher.trigger(events::Update{timeDelta});
	dispatcher.trigger(events::FrameEnd{timeDelta});

	dispatcher.trigger<events::Start>();
	dispatcher.sink<events::Start>().disconnect();

	graphics.draw();
}

static const int step     = 1;
static const int fastStep = 100;

static const float fStep     = 0.01f;
static const float fFastStep = fStep*100;

void Gui::update(const events::Update&)
{
	ImGui::Begin("FDTD");

	ImGui::SeparatorText("Simulation parameters");

	basicTab();

	ImGui::SeparatorText("Backends");

	utils::imgui::Combo<Backend>("Backend type", &fdtdInfo.backend);
	utils::imgui::Combo<Precision>("Precision", &fdtdInfo.precision);

	if(ImGui::Button("Start"))
	{
	}

	ImGui::End();

	ImPlot3D::ShowDemoWindow();

	// This is broken
	//ImGui::End();

	//ImGui::Begin("Service graph");

	//ImguiGraphnodeEdgeWriter writer;
	//_injector.printEdges(writer);

	//ImGui::End();
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

void Gui::basicTab()
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

Gui::~Gui()
{
	dispatcher.sink<events::Update>().disconnect<&Gui::update>(*this);
}

}
