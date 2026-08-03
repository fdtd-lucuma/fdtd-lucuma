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

export module lucuma.services.frontends:gui;

import lucuma.utils;
import lucuma.services.window;
import lucuma.services.vulkan;
import lucuma.services.basic;
import lucuma.legacy_headers.entt;
import lucuma.events;

import std;

namespace lucuma::services::frontends
{

using namespace lucuma::utils;

export class Gui
{
public:
	Gui(Injector& injector);

	void start();

private:
	Injector& _injector;

	entt::dispatcher&     dispatcher;
	entt::registry&       registry;
	window::Sdl3&         glfw;
	vulkan::Graphics&     graphics;
	vulkan::Imgui&        imgui;
	basic::Settings&      settings;
	basic::Systems&       systems;

	void drawFrame(float timeDelta);

};

}
