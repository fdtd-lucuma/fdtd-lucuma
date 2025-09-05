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

#include <cassert>

module fdtd.services.vulkan;

VulkanDevice::VulkanDevice([[maybe_unused]] Injector& injector):
	vulkanCore(injector.inject<VulkanCore>())
{
	init();
}

vk::raii::PhysicalDevice& VulkanDevice::getPhysicalDevice()
{
	return vulkanCore.getPhysicalDevice();
}

vk::raii::Device& VulkanDevice::getDevice()
{
	return device;
}


vk::raii::Queue& VulkanDevice::getComputeQueue()
{
	assert(!computeQueues.empty());

	return computeQueues[0];
}

void VulkanDevice::init()
{
	createDevice();
}

QueueFamilyInfo VulkanDevice::selectComputeQueueFamily(std::span<const vk::QueueFamilyProperties> properties)
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

vk::DeviceQueueCreateInfo VulkanDevice::getComputeQueueCreateInfo(std::span<const vk::QueueFamilyProperties> properties)
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

std::vector<vk::DeviceQueueCreateInfo> VulkanDevice::getQueueCreateInfos()
{
	auto queueProperties = getPhysicalDevice().getQueueFamilyProperties();

	return {
		getComputeQueueCreateInfo(queueProperties)
	};
}

void VulkanDevice::createDevice()
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

	computeQueues = createQueues(computeQueueInfo);
}

std::vector<const char*> VulkanDevice::getRequiredLayers()
{
	return {};
}

std::vector<const char*> VulkanDevice::getRequiredExtensions()
{
	return {};
}

std::vector<vk::raii::Queue> VulkanDevice::createQueues(const QueueFamilyInfo& info)
{
	return
		std::views::iota(0u, info.count) |
		std::views::transform([&](auto i){return device.getQueue(info.index, i);}) |
		std::ranges::to<std::vector>();
}
