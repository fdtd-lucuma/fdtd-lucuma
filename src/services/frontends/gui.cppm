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

struct FdtdInfo
{
	// Basic
	float basicSize[3];
	float basicGaussPosition[3];
	float basicTime;

	// Advanced
	unsigned int size[3];
	unsigned int gaussPosition[3];
	float        deltaT;
	float        imp0;
	float        Cr;
	unsigned int maxTime;
	float        gaussSigma;

	Backend   backend;
	Precision precision;
};

export class Gui
{
public:
	Gui(Injector& injector);

	void init();
	void start();

	~Gui();

private:
	Injector& _injector;

	entt::dispatcher&     dispatcher;
	entt::registry&       registry;
	window::Glfw&         glfw;
	vulkan::Graphics&     graphics;
	vulkan::TriangleDemo& triangleDemo;
	vulkan::Imgui&        imgui;
	basic::Settings&      settings;

	void drawFrame(float timeDelta);

	void update(const events::Update& event);

	void basicTab();
	void advancedTab();

	FdtdInfo fdtdInfo;

};

}
