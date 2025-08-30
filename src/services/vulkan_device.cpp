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

module fdtd.services;

import std;

VulkanDevice::VulkanDevice([[maybe_unused]] Injector& injector):
	vulkanCore(injector.inject<VulkanCore>())
{
	init();
}

vk::raii::PhysicalDevice& VulkanDevice::getPhysicalDevice()
{
	return physicalDevice;
}

vk::raii::Device& VulkanDevice::getDevice()
{
	return device;
}

void VulkanDevice::init()
{
	createDevices();
}

void VulkanDevice::createDevices()
{
	physicalDevice = selectPhysicalDevice();

	constexpr vk::DeviceCreateInfo deviceCreateInfo{};

	device = physicalDevice.createDevice(deviceCreateInfo);
}

vk::raii::PhysicalDevice VulkanDevice::selectPhysicalDevice()
{
	auto filter = [this](const auto& x){return isSuitable(x);};

	// TODO: Force from command line arguments
	for(const auto& device:
		vulkanCore.getInstance().enumeratePhysicalDevices() |
		std::views::filter(filter)
	)
	{
		return device;
	}

	throw std::runtime_error("Failed to find a Vulkan compatible GPU");
}

bool VulkanDevice::isSuitable(vk::PhysicalDevice physicalDevice)
{
	auto properties = physicalDevice.getProperties();
	//auto features   = physicalDevice.getFeatures();

	if(properties.apiVersion < vk::ApiVersion14)
		return false;

	// TODO: Check for extensions

	return true;
}
