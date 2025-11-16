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

void Gui::update(const events::Update&)
{
	ImGui::Begin("FDTD");

	ImGui::SeparatorText("Simulation parameters");

	static const int step     = 1;
	static const int fastStep = 100;

	ImGui::InputScalarN("Size", ImGuiDataType_U32, fdtdInfo.size, 3, &step, &fastStep);
	ImGui::InputScalarN("Source position", ImGuiDataType_U32, fdtdInfo.gaussPosition, 3, &step, &fastStep);
	ImGui::InputFloat("DeltaT", &fdtdInfo.deltaT);
	ImGui::InputFloat("Imp0", &fdtdInfo.imp0);
	ImGui::InputFloat("Cr", &fdtdInfo.Cr);
	ImGui::InputScalar("Time steps", ImGuiDataType_U32, &fdtdInfo.maxTime, &step, &fastStep);
	ImGui::InputFloat("Gaussian sigma", &fdtdInfo.gaussSigma);

	ImGui::SeparatorText("Backends");

	utils::imgui::Combo<Backend>("Backend type", &fdtdInfo.backend);
	utils::imgui::Combo<Precision>("Precision", &fdtdInfo.precision);

	if(ImGui::Button("Start"))
	{
	}

	// This is broken
	//ImGui::End();

	//ImGui::Begin("Service graph");

	//ImguiGraphnodeEdgeWriter writer;
	//_injector.printEdges(writer);

	//ImGui::End();
}

Gui::~Gui()
{
	dispatcher.sink<events::Update>().disconnect<&Gui::update>(*this);
}

}
