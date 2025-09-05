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

import std;
import fdtd.utils;
import fdtd.services;
import fdtd.services.vulkan;

Simulator::Simulator()
{}

int Simulator::run(int argc, char** argv)
{
	Injector injector;

	injector.emplace<ArgumentParser>(argc, argv);
	injector.emplace<VulkanAll>(injector);
	auto& builder = injector.emplace<VulkanPipelineBuilder>(injector);

	auto pipeline = builder.createComputePipeline({
		.shaderPath = "./test.spv",
	});

	return EXIT_SUCCESS;
}
