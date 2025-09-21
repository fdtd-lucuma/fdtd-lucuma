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

export module fdtd.services:compute;

import fdtd.utils;
import fdtd.services.vulkan;

import std;

namespace fdtd::services
{

using namespace fdtd::utils;

export class Compute
{
public:
	Compute(Injector& injector);

	void compute();

private:
	struct HelloWorldData
	{
		vulkan::ComputePipeline pipeline;

		vulkan::Buffer aBuffer;
		vulkan::Buffer bBuffer;
		vulkan::Buffer cBuffer;
	};

	vulkan::Allocator& vulkanAllocator;
	vulkan::Compute&   vulkanCompute;

	HelloWorldData createHelloWorld(std::size_t n);

};

}
