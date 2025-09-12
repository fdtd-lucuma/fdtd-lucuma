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

Device::Device([[maybe_unused]] Injector& injector):
	core(injector.inject<Core>()),
	settings(injector.inject<basic::Settings>())

{
	init();
}

vk::raii::PhysicalDevice& Device::getPhysicalDevice()
{
	return core.getPhysicalDevice();
}

vk::raii::Device& Device::getDevice()
{
	return device;
}


vk::raii::Queue& Device::getComputeQueue()
{
	return computeQueues[0];
}

vk::raii::CommandPool& Device::getComputeCommandPool()
{
	return computeCommandPool;
}

void Device::init()
{
	createDevice();
}

QueueFamilyInfo Device::selectComputeQueueFamily(std::span<const vk::QueueFamilyProperties> properties)
{
	auto filter = [&](std::uint32_t i) {return (bool)(properties[i].queueFlags & vk::QueueFlagBits::eCompute); };

	std::uint32_t maxCount = std::numeric_limits<std::uint32_t>::lowest();
	std::uint32_t maxIndex;

	for(std::uint32_t i: std::views::iota(0u, (std::uint32_t)properties.size()) | std::views::filter(filter))
	{
		if(properties[i].queueCount > maxCount)
		{
			maxCount = properties[i].queueCount;
			maxIndex = i;
		}
	}

	if(maxCount == std::numeric_limits<std::uint32_t>::lowest())
		throw new std::runtime_error("Couldn't find a compute queue family.");

	return {
		.index = maxIndex,
		.count = maxCount,
	};
}

vk::DeviceQueueCreateInfo Device::getComputeQueueCreateInfo(std::span<const vk::QueueFamilyProperties> properties)
{
	computeQueueInfo = selectComputeQueueFamily(properties);

	std::cout
		<< "Selected queue family for compute:\n"
		<< "Queue family " << computeQueueInfo.index << ":\n"
		<< properties[computeQueueInfo.index]
	;

	vk::DeviceQueueCreateInfo computeQueueCreateInfo {
		.queueFamilyIndex = computeQueueInfo.index,
	};

	computeQueueCreateInfo.setQueuePriorities(computeQueueInfo.priorities);

	return computeQueueCreateInfo;
}

std::vector<vk::DeviceQueueCreateInfo> Device::getQueueCreateInfos()
{
	auto queueProperties = getPhysicalDevice().getQueueFamilyProperties();

	std::vector<vk::DeviceQueueCreateInfo> createInfos;

	createInfos.emplace_back(getComputeQueueCreateInfo(queueProperties));

	if(!settings.isHeadless())
	{
		// TODO: getGraphicsQueueCreateInfo
	}

	return createInfos;
}

void Device::createDevice()
{
	//auto features   = getPhysicalDevice().getFeatures();

	auto extensions = getRequiredExtensions();
	auto layers     = getRequiredLayers();

	auto queues = getQueueCreateInfos();

	vk::DeviceCreateInfo deviceCreateInfo{};

	deviceCreateInfo
		.setPEnabledLayerNames(layers)
		.setPEnabledExtensionNames(extensions)
		.setQueueCreateInfos(queues)
	;

	device = getPhysicalDevice().createDevice(deviceCreateInfo);

	computeQueues      = createQueues(computeQueueInfo);
	computeCommandPool = createCommandPool(computeQueueInfo);

	if(!settings.isHeadless())
	{
		// TODO: Init graphics stuff
	}
}

std::vector<const char*> Device::getRequiredLayers()
{
	return {};
}

std::vector<const char*> Device::getRequiredExtensions()
{
	return {};
}

std::vector<vk::raii::Queue> Device::createQueues(const QueueFamilyInfo& info)
{
	return
		std::views::iota(0u, info.count) |
		std::views::transform([&](auto i){return device.getQueue(info.index, i);}) |
		std::ranges::to<std::vector>();
}

vk::raii::CommandPool Device::createCommandPool(const QueueFamilyInfo& info)
{
	vk::CommandPoolCreateInfo createInfo {
		.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = info.index,
	};

	return getDevice().createCommandPool(createInfo);
}

}
