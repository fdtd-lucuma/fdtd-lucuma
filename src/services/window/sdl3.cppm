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

export module lucuma.services.window:sdl3;

import std;
import vulkan;

import lucuma.utils;
import lucuma.services.basic;
import lucuma.legacy_headers.sdl3;

namespace lucuma::services::window
{

using namespace lucuma::utils;
using namespace lucuma::services;

export class Sdl3
{
public:
	Sdl3(Injector& injector);
	~Sdl3();

	bool shouldClose() const;
	void pollEvents();

	void waitUntilMaximixed();

	std::tuple<std::size_t, std::size_t> getFramebufferSize() const noexcept;

	void initImgui();
	void shutdownImgui();
	void newImguiFrame();

	vk::raii::SurfaceKHR createSurface(const vk::raii::Instance& instance);

	void setOnFrameBufferResize(std::function<void(std::size_t, std::size_t)>&& func);

	void appendExtensions(std::vector<const char*>& v) const;

private:
	basic::Settings& settings;

	sdl3::unique_window window = nullptr;

	void init();
	std::span<const char* const> getExtensions() const noexcept;

	bool _shouldClose = false;
	std::function<void(std::size_t, std::size_t)> frameBufferResizeCallback;

};

}
