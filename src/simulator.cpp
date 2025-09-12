// Una GUI para fdtd
// Copyright © 2025 Otreblan
//
// fdtd-vulkan is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// fdtd-vulkan is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fdtd-vulkan.  If not, see <http://www.gnu.org/licenses/>.

module;

#include <cstdlib>

module fdtd;

import fdtd.utils;
import fdtd.services;
import std.compat;

namespace fdtd
{

Simulator::Simulator()
{}

int Simulator::run(int argc, char** argv)
{
	utils::Injector injector;

	auto& arguments = injector.emplace<services::basic::ArgumentParser>(argc, argv);
	injector.emplace<services::vulkan::All>(injector);

	auto& settings = injector.inject<services::basic::Settings>();

	if(settings.isHeadless())
	{
		auto& compute = injector.inject<services::Compute>();

		compute.compute();
	}
	else // Gui
	{
		// TODO: Init gui or headless
	}

	auto& graphPath = arguments.graphPath();

	if(graphPath.has_value())
		injector.printEdges(*graphPath);

	return EXIT_SUCCESS;
}

}
