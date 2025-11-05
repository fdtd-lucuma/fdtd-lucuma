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

export module lucuma.services.vulkan:graphics;

import vulkan_hpp;
import std;

import lucuma.utils;
import lucuma.utils.vulkan;

namespace lucuma::services::vulkan
{

using namespace lucuma::utils;
using namespace lucuma::utils::vulkan;
using namespace lucuma::services;

class Buffer;
class Device;
class ShaderLoader;
class Swapchain;


export class Graphics
{
public:
	Graphics(Injector& injector);

	vk::raii::Queue&       getQueue();
	vk::raii::CommandPool& getCommandPool();

private:
	Device&       device;
	ShaderLoader& shaderLoader;
	Swapchain&    swapchain;

	std::vector<vk::raii::Queue> queues;
	vk::raii::CommandPool		commandPool = nullptr;

	void init();

	void createQueues();
	void createCommandPool();
};

}
