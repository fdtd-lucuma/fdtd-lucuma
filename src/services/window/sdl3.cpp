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

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_init.h>
#include <vulkan/vulkan_core.h>

module lucuma.services.window;

import imgui;
import imgui_impl_sdl3;
import imgui_impl_vulkan;

import lucuma.services.basic;

namespace lucuma::services::window
{

using namespace lucuma::services;


Sdl3::Sdl3([[maybe_unused]] Injector& injector):
	settings(injector.inject<basic::Settings>())
{
	init();
}

void error_callback(int, const char *description) {
	std::print(std::cerr, "SDL3: {}\n", description);
}

void Sdl3::init()
{
	SDL_Init(
		SDL_INIT_AUDIO |
		SDL_INIT_VIDEO |
		SDL_INIT_JOYSTICK |
		SDL_INIT_HAPTIC |
		SDL_INIT_GAMEPAD |
		SDL_INIT_EVENTS
	);

	SDL_SetAppMetadata("fdtd-lucuma", "1.0.0", "fdtd-lucuma");

	window.reset(SDL_CreateWindow("fdtd-lucuma", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY));
}


bool Sdl3::shouldClose() const
{
	return _shouldClose;
}

void Sdl3::pollEvents()
{
	SDL_Event event;

	while(SDL_PollEvent(&event))
	{
		ImGui_ImplSDL3_ProcessEvent(&event);
		switch(event.type)
		{
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				if(event.window.windowID != SDL_GetWindowID(window.get()))
					break;
				[[fallthrough]];
			case SDL_EVENT_QUIT:
				_shouldClose = true;
				break;
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
				if(frameBufferResizeCallback)
				{
					auto e = event.window;
					frameBufferResizeCallback(e.data1, e.data2);
				}
				break;
		}
	}
}

void Sdl3::waitUntilMaximixed()
{
	auto [width, height] = getFramebufferSize();

	while(width == 0 || height == 0)
	{
		std::tie(width, height) = getFramebufferSize();

		SDL_WaitEvent(nullptr);
	}

}

std::tuple<std::size_t, std::size_t> Sdl3::getFramebufferSize() const noexcept
{
	int width, height;
	SDL_GetWindowSizeInPixels(window.get(), &width, &height);
	return {width, height};
}

void Sdl3::initImgui()
{
	ImGui_ImplSDL3_InitForVulkan(window.get());
}

void Sdl3::shutdownImgui()
{
	ImGui_ImplSDL3_Shutdown();
}

void Sdl3::newImguiFrame()
{
	ImGui_ImplSDL3_NewFrame();
}

vk::raii::SurfaceKHR Sdl3::createSurface(const vk::raii::Instance& instance)
{
	VkSurfaceKHR surface;
	SDL_Vulkan_CreateSurface(window.get(), *instance, nullptr, &surface);

	return vk::raii::SurfaceKHR(instance, surface);
}

void Sdl3::setOnFrameBufferResize(std::function<void(std::size_t, std::size_t)>&& func)
{
	frameBufferResizeCallback = func;
}

void Sdl3::appendExtensions(std::vector<const char*>& v) const
{
	v.append_range(getExtensions());
}

std::span<const char* const> Sdl3::getExtensions() const noexcept
{
	std::uint32_t count;
	auto extensions = SDL_Vulkan_GetInstanceExtensions(&count);

	return std::span<const char* const>(extensions, (std::size_t)count);
}

Sdl3::~Sdl3()
{
	SDL_Quit();
}

}
