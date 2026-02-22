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

import lucuma.services.basic;

namespace lucuma::services::window
{

using namespace lucuma::services;

Glfw::Glfw([[maybe_unused]] Injector& injector):
	settings(injector.inject<basic::Settings>())

{
	init();
}

void error_callback(int, const char *description) {
	std::print(std::cerr, "GLFW: {}\n", description);
}

void Glfw::init()
{
	vkfw::setErrorCallback(error_callback);
	instance = vkfw::initUnique();

	vkfw::windowHint<vkfw::WindowHint::eClientAPI>(vkfw::ClientAPI::eNone);
	vkfw::windowHint<vkfw::WindowHint::eSRGBCapable>(true);
	vkfw::windowHint<vkfw::WindowHint::eResizable>(true);

	window = vkfw::createWindowUnique(800, 600, "fdtd-lucuma");

	// TODO: Key callback
}


bool Glfw::shouldClose() const
{
	return window->shouldClose();
}

void Glfw::pollEvents()
{
	vkfw::pollEvents();
}

vkfw::Window& Glfw::getWindow()
{
	return window.get();
}

void Glfw::waitUntilMaximixed()
{
	auto [width, height] = window->getFramebufferSize();

	while(width == 0 || height == 0)
	{
		std::tie(width, height) = window->getFramebufferSize();

		vkfw::waitEvents();
	}

}

}
