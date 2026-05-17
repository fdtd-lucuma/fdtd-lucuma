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

#include <cstdlib>

module lucuma;

import lucuma.utils;
import lucuma.services;
import std.compat;
import magic_enum;
import lucuma.legacy_headers.entt;

namespace lucuma
{

Simulator::Simulator()
{}

int Simulator::run(int argc, char** argv)
{
	initBasic(argc, argv);
	selectFrontend();
	printGraph();

	return EXIT_SUCCESS;
}

void Simulator::initBasic(int argc, char** argv)
{
	injector.emplace<services::basic::ArgumentParser>(argc, argv);
	auto& settings = injector.inject<services::basic::Settings>();

	if(settings.tracy())
	{
		injector.emplace<services::basic::Tracy>(injector);
	}
}

void Simulator::selectFrontend()
{
	auto& settings     = injector.inject<services::basic::Settings>();
	auto& instantiator = injector.inject<services::backends::Instantiator>();

	if(settings.isHeadless())
	{
		instantiator.instantiate();
		auto& headless = injector.inject<services::frontends::Headless>();

		headless.compute();
	}
	else // Gui
	{
		instantiator.instantiateAll();

		auto& gui = injector.inject<services::frontends::Gui>();

		gui.start();
	}
}

void Simulator::printGraph()
{
	auto& graphPath = injector.inject<services::basic::ArgumentParser>().graphPath();

	if(graphPath.has_value())
		injector.printEdges(*graphPath);
}

}
