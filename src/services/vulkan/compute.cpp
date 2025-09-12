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

module fdtd.services.vulkan;

import fdtd.services.basic;

namespace fdtd::services::vulkan
{

using namespace fdtd::services;

Compute::Compute([[maybe_unused]] Injector& injector):
	device(injector.inject<Device>())

{
	if(!device.getComputeInfo().has_value())
	{
		throw new std::runtime_error("Couldn't find a compute queue family.");
	}

	init();
}

vk::raii::Queue& Compute::getQueue()
{
	return queues[0];
}

vk::raii::CommandPool& Compute::getCommandPool()
{
	return commandPool;
}

void Compute::init()
{
	createQueues();
	createCommandPool();
}

void Compute::createQueues()
{
	auto info = device.getComputeInfo();

	queues =
		std::views::iota(0u, info->count) |
		std::views::transform([&](auto i){return device.getDevice().getQueue(info->index, i);}) |
		std::ranges::to<std::vector>();
}

void Compute::createCommandPool()
{
	auto info = device.getComputeInfo();

	vk::CommandPoolCreateInfo createInfo {
		.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = info->index,
	};

	commandPool = device.getDevice().createCommandPool(createInfo);
}

}
