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
import lucuma.services.window;
import lucuma.services.vulkan;
import lucuma.legacy_headers.entt;
import lucuma.events;

import imgui;

import std;

namespace lucuma::services::frontends
{

Gui::Gui([[maybe_unused]]Injector& injector):
	dispatcher(injector.inject<entt::dispatcher>()),
	registry(injector.inject<entt::registry>()),
	glfw(injector.inject<window::Glfw>()),
	graphics(injector.inject<vulkan::Graphics>()),
	triangleDemo(injector.inject<vulkan::TriangleDemo>()),
	imgui(injector.inject<vulkan::Imgui>())
{
	init();
}

void Gui::init()
{
	// TODO: Wrap this in a raii helper
	dispatcher.sink<events::Update>().connect<&Gui::update>(*this);
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

	graphics.draw();
}

void Gui::update(const events::Update&)
{
	ImGui::ShowDemoWindow();
}

Gui::~Gui()
{
	dispatcher.sink<events::Update>().disconnect<&Gui::update>(*this);
}

}
