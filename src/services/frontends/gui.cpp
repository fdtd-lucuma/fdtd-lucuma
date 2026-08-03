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

module lucuma.services.frontends;

import lucuma.utils;
import lucuma.services.window;
import lucuma.services.vulkan;
import lucuma.legacy_headers.entt;
import lucuma.events;
import lucuma.systems;

import std;

namespace lucuma::services::frontends
{

Gui::Gui([[maybe_unused]]Injector& injector):
	_injector(injector),
	dispatcher(injector.inject<entt::dispatcher>()),
	registry(injector.inject<entt::registry>()),
	glfw(injector.inject<window::Sdl3>()),
	graphics(injector.inject<vulkan::Graphics>()),
	imgui(injector.inject<vulkan::Imgui>()),
	settings(injector.inject<basic::Settings>()),
	systems(injector.inject<basic::Systems>())
{ }

void Gui::start()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	auto lastTime = currentTime;

	float timeDelta = 1.f/60;

	systems.start<systems::Root>();

	while(!glfw.shouldClose())
	{
		lastTime = currentTime;

		glfw.pollEvents();

		drawFrame(timeDelta);

		if(settings.tracy())
			FrameMark;

		currentTime = std::chrono::high_resolution_clock::now();
		timeDelta   = std::chrono::duration<float, std::chrono::seconds::period>(currentTime-lastTime).count();
	}
}

void Gui::drawFrame(float timeDelta)
{
	{
		ZoneNamedN(__zone, "Start", settings.tracy());
		dispatcher.trigger(events::FrameStart{timeDelta});
	}
	{
		ZoneNamedN(__zone, "Update", settings.tracy());
		dispatcher.trigger(events::Update{timeDelta});
	}
	{
		ZoneNamedN(__zone, "Post update", settings.tracy());
		dispatcher.trigger(events::PostUpdate{timeDelta});
	}
	{
		ZoneNamedN(__zone, "End", settings.tracy());
		dispatcher.trigger(events::FrameEnd{timeDelta});
	}

	{
		ZoneNamedN(__zone, "Clean stopped", settings.tracy());
		systems.cleanStopped(); //TODO: Move this into frameEnd
	}

	{
		ZoneNamedN(__zone, "System starts", settings.tracy());

		dispatcher.trigger<events::Start>();
		dispatcher.sink<events::Start>().disconnect();
	}

	{
		ZoneNamedN(__zone, "Draw", settings.tracy());
		graphics.draw();
	}
}

}
