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

module lucuma.services.vulkan;

import lucuma.services.basic;

namespace lucuma::services::vulkan
{

using namespace lucuma::services;

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

const vk::raii::PhysicalDevice& Device::getPhysicalDevice() const
{
	return core.getPhysicalDevice();
}

vk::raii::Device& Device::getDevice()
{
	return device;
}

const vk::raii::Device& Device::getDevice() const
{
	return device;
}


const std::optional<QueueFamilyInfo>& Device::getGraphicsInfo() const
{
	return graphicsInfo;
}

const std::optional<QueueFamilyInfo>& Device::getComputeInfo() const
{
	return computeInfo;
}

const std::optional<QueueFamilyInfo>& Device::getPresentInfo() const
{
	return presentInfo;
}

void Device::init()
{
	createDevice();
}

QueueFamilyInfo Device::selectQueueFamilyCommon(std::span<const vk::QueueFamilyProperties> properties, vk::QueueFlags queueFlags)
{
	auto filter = [&](auto&& i) {return (bool)(properties[i].queueFlags & queueFlags); };

	std::uint32_t maxCount = std::numeric_limits<std::uint32_t>::lowest();
	std::uint32_t maxIndex;

	for(std::uint32_t i: std::views::iota(0zu, properties.size()) | std::views::filter(filter))
	{
		if(properties[i].queueCount > maxCount)
		{
			maxCount = properties[i].queueCount;
			maxIndex = i;

			if(settings.tracy() && settings.isHeadless()) // For some reason in amd only the first queue family has performance counters.
				break;
		}
	}

	if(maxCount == std::numeric_limits<std::uint32_t>::lowest())
		throw new std::runtime_error(std::format("Couldn't find a {} queue family.", to_string(queueFlags)));

	return {
		.index = maxIndex,
		.count = maxCount,
	};
}

QueueFamilyInfo Device::selectComputeQueueFamily(std::span<const vk::QueueFamilyProperties> properties)
{
	return selectQueueFamilyCommon(properties, vk::QueueFlagBits::eCompute);
}

QueueFamilyInfo Device::selectGraphicsQueueFamily(std::span<const vk::QueueFamilyProperties> properties)
{
	return selectQueueFamilyCommon(properties, vk::QueueFlagBits::eGraphics);
}

vk::DeviceQueueCreateInfo Device::getComputeQueueCreateInfo(std::span<const vk::QueueFamilyProperties> properties)
{
	computeInfo.emplace(selectComputeQueueFamily(properties));

	if(settings.debug())
	{
		std::cout
			<< "Selected queue family for compute:\n"
			<< "Queue family " << computeInfo->index << ":\n"
			<< properties[computeInfo->index]
			;
	}

	vk::DeviceQueueCreateInfo computeQueueCreateInfo {
		.queueFamilyIndex = computeInfo->index,
	};

	computeQueueCreateInfo.setQueuePriorities(computeInfo->priorities);

	return computeQueueCreateInfo;
}

vk::DeviceQueueCreateInfo Device::getGraphicsQueueCreateInfo(std::span<const vk::QueueFamilyProperties> properties)
{
	graphicsInfo.emplace(selectGraphicsQueueFamily(properties));

	if(!core.getPhysicalDevice().getSurfaceSupportKHR(graphicsInfo->index, core.getSurface()))
		throw new std::runtime_error("Graphics queue doesn't support present.");

	presentInfo = graphicsInfo; //TODO: support graphics != present?

	if(settings.debug())
	{
		std::cout
			<< "Selected queue family for graphics:\n"
			<< "Queue family " << graphicsInfo->index << ":\n"
			<< properties[graphicsInfo->index]
			;
	}

	vk::DeviceQueueCreateInfo graphicsQueueCreateInfo {
		.queueFamilyIndex = graphicsInfo->index,
	};

	graphicsQueueCreateInfo.setQueuePriorities(graphicsInfo->priorities);

	return graphicsQueueCreateInfo;
}

std::vector<vk::DeviceQueueCreateInfo> Device::getQueueCreateInfos()
{
	auto queueProperties = getPhysicalDevice().getQueueFamilyProperties();

	std::vector<vk::DeviceQueueCreateInfo> createInfos;

	createInfos.emplace_back(getComputeQueueCreateInfo(queueProperties));

	if(!settings.isHeadless())
	{
		createInfos.emplace_back(getGraphicsQueueCreateInfo(queueProperties));
	}

	return createInfos;
}

void Device::createDevice()
{

	auto extensions = getRequiredExtensions();

	auto queues = getQueueCreateInfos();

	auto chain = getPhysicalDevice().getFeatures2<
		vk::DeviceCreateInfo,
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features
#ifndef BUILD_FOR_TERMUX
		,
		vk::PhysicalDeviceVulkan12Features,
		vk::PhysicalDeviceVulkan13Features
#endif
		//vk::PhysicalDeviceVulkan14Features
	>();

	auto& deviceCreateInfo = chain.get<vk::DeviceCreateInfo>();

	deviceCreateInfo
		.setPEnabledExtensionNames(extensions)
		.setQueueCreateInfos(queues)
	;

	device = getPhysicalDevice().createDevice(deviceCreateInfo);
}

std::vector<const char*> Device::getRequiredExtensions()
{
	std::vector<const char*> result;

	if(!settings.isHeadless())
		result.emplace_back(vk::KHRSwapchainExtensionName);

	if(settings.tracy())
		result.emplace_back(vk::KHRPerformanceQueryExtensionName);

	return result;
}


void Device::waitIdle() const
{
	getDevice().waitIdle();
}
}
