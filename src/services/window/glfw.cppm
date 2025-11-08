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

export module lucuma.services.window:glfw;

import std;
import vkfw;

import lucuma.utils;
import lucuma.services.basic;

namespace lucuma::services::window
{

using namespace lucuma::utils;
using namespace lucuma::services;

export class Glfw
{
public:
	Glfw(Injector& injector);

	bool shouldClose() const;
	void pollEvents();

	vkfw::Window& getWindow();

	void waitUntilMaximixed();

private:
	basic::Settings& settings;

	vkfw::UniqueInstance instance;
	vkfw::UniqueWindow   window;

	void init();
};

}
