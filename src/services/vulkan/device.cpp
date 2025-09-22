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

vk::raii::Device& Device::getDevice()
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

void Device::init()
{
	createDevice();
}

QueueFamilyInfo Device::selectComputeQueueFamily(std::span<const vk::QueueFamilyProperties> properties)
{
	auto filter = [&](auto&& i) {return (bool)(properties[i].queueFlags & vk::QueueFlagBits::eCompute); };

	std::uint32_t maxCount = std::numeric_limits<std::uint32_t>::lowest();
	std::uint32_t maxIndex;

	for(std::uint32_t i: std::views::iota(0zu, properties.size()) | std::views::filter(filter))
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
	computeInfo.emplace(selectComputeQueueFamily(properties));

	std::cout
		<< "Selected queue family for compute:\n"
		<< "Queue family " << computeInfo->index << ":\n"
		<< properties[computeInfo->index]
	;

	vk::DeviceQueueCreateInfo computeQueueCreateInfo {
		.queueFamilyIndex = computeInfo->index,
	};

	computeQueueCreateInfo.setQueuePriorities(computeInfo->priorities);

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
	// TODO: Chain 1.1 and 1.2 features
	auto features   = getPhysicalDevice().getFeatures();

	auto extensions = getRequiredExtensions();
	auto layers     = getRequiredLayers();

	auto queues = getQueueCreateInfos();

	vk::DeviceCreateInfo deviceCreateInfo{};

	deviceCreateInfo
		.setPEnabledLayerNames(layers)
		.setPEnabledExtensionNames(extensions)
		.setQueueCreateInfos(queues)
		.setPEnabledFeatures(&features)
	;

	device = getPhysicalDevice().createDevice(deviceCreateInfo);
}

std::vector<const char*> Device::getRequiredLayers()
{
	return {};
}

std::vector<const char*> Device::getRequiredExtensions()
{
	return {};
}

}
