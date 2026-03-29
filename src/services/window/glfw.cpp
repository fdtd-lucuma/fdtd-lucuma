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

import vkfw;
import imgui;
import imgui_impl_glfw;
import imgui_impl_vulkan;

import lucuma.services.basic;

namespace lucuma::services::window
{

using namespace lucuma::services;


struct Glfw::Data
{
	vkfw::UniqueInstance instance;
	vkfw::UniqueWindow   window;
};

Glfw::Glfw([[maybe_unused]] Injector& injector):
	settings(injector.inject<basic::Settings>()),
	_data(std::in_place_type<Glfw::Data>)

{
	init();
}

void error_callback(int, const char *description) {
	std::print(std::cerr, "GLFW: {}\n", description);
}

void Glfw::init()
{
	vkfw::setErrorCallback(error_callback);
	data().instance = vkfw::initUnique();

	vkfw::windowHint<vkfw::WindowHint::eClientAPI>(vkfw::ClientAPI::eNone);
	vkfw::windowHint<vkfw::WindowHint::eSRGBCapable>(true);
	vkfw::windowHint<vkfw::WindowHint::eResizable>(true);

	data().window = vkfw::createWindowUnique(800, 600, "fdtd-lucuma");

	// TODO: Key callback
}


bool Glfw::shouldClose() const
{
	return data().window->shouldClose();
}

void Glfw::pollEvents()
{
	vkfw::pollEvents();
}

void Glfw::waitUntilMaximixed()
{
	auto [width, height] = data().window->getFramebufferSize();

	while(width == 0 || height == 0)
	{
		std::tie(width, height) = data().window->getFramebufferSize();

		vkfw::waitEvents();
	}

}

Glfw::Data& Glfw::data()
{
	return *(Glfw::Data*)_data.data();
}

const Glfw::Data& Glfw::data() const
{
	return *(Glfw::Data*)_data.data();
}

std::tuple<std::size_t, std::size_t> Glfw::getFramebufferSize() const
{
	return data().window->getFramebufferSize();
}

void Glfw::initImgui()
{
	ImGui_ImplGlfw_InitForVulkan(data().window.get(), true);
}

void Glfw::shutdownImgui()
{
	ImGui_ImplGlfw_Shutdown();
}

void Glfw::newImguiFrame()
{
	ImGui_ImplGlfw_NewFrame();
}

vk::raii::SurfaceKHR Glfw::createSurface(const vk::raii::Instance& instance)
{
	return vk::raii::SurfaceKHR(instance, vkfw::createWindowSurface(instance, data().window.get()));
}

void Glfw::setOnFrameBufferResize(std::function<void(std::size_t, std::size_t)>&& func)
{
	data().window->callbacks()->on_framebuffer_resize = [=](const vkfw::Window&, std::size_t x, std::size_t y)
	{
		func(x, y);
	};
}

}
