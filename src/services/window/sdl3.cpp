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
	//TODO
	//vkfw::setErrorCallback(error_callback);
	//data().instance = vkfw::initUnique();

	//vkfw::windowHint<vkfw::WindowHint::eClientAPI>(vkfw::ClientAPI::eNone);
	//vkfw::windowHint<vkfw::WindowHint::eSRGBCapable>(true);
	//vkfw::windowHint<vkfw::WindowHint::eResizable>(true);

	//data().window = vkfw::createWindowUnique(800, 600, "fdtd-lucuma");

	// TODO: Key callback
}


bool Sdl3::shouldClose() const
{
	//TODO
	return true;
	//return data().window->shouldClose();
}

void Sdl3::pollEvents()
{
	//TODO
	//vkfw::pollEvents();
}

void Sdl3::waitUntilMaximixed()
{
	//TODO
	//auto [width, height] = data().window->getFramebufferSize();

	//while(width == 0 || height == 0)
	//{
	//	std::tie(width, height) = data().window->getFramebufferSize();

	//	vkfw::waitEvents();
	//}

}

std::tuple<std::size_t, std::size_t> Sdl3::getFramebufferSize() const
{
	//return data().window->getFramebufferSize();
	return {1,1};
}

void Sdl3::initImgui()
{
	//TODO
	//ImGui_ImplSdl3_InitForVulkan(data().window.get(), true);
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
	// TODO
	//return vk::raii::SurfaceKHR(instance, vkfw::createWindowSurface(instance, data().window.get()));
}

void Sdl3::setOnFrameBufferResize(std::function<void(std::size_t, std::size_t)>&& func)
{
	// TODO
	//data().window->callbacks()->on_framebuffer_resize = [=](const vkfw::Window&, std::size_t x, std::size_t y)
	//{
	//	func(x, y);
	//};
}

}
