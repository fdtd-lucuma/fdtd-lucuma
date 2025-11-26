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

export module lucuma.services.vulkan:imgui;

import vulkan_hpp;
import std;

import lucuma.utils;
import lucuma.utils.vulkan;
import lucuma.services.window;
import lucuma.services.basic;
import lucuma.legacy_headers.entt;
import lucuma.events.vulkan;
import lucuma.events;

import imgui;

namespace lucuma::services::vulkan
{

using namespace lucuma::utils;
using namespace lucuma::utils::vulkan;
using namespace lucuma::services;

class Device;
class Core;
class Swapchain;
class Graphics;

export class Imgui
{
public:
	Imgui(Injector& injector);

	~Imgui();

private:
	entt::dispatcher& dispatcher;
	entt::registry&   registry;
	Core&             core;
	Device&           device;
	Swapchain&        swapchain;
	Graphics&         graphics;
	window::Glfw&     glfw;
	basic::XdgDirs&   xdgDirs;

	std::filesystem::path iniPath;

	void init();

	void initImgui();

	void onDraw(const events::vulkan::GuiDraw& event);
	void onFrameStart(const events::FrameStart& event);
	void onFrameEnd(const events::FrameEnd& event);

};

}
