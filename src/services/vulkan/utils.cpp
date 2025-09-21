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

import vulkan_hpp;
import std;

namespace lucuma::services::vulkan
{

void list_vulkan_extensions()
{
	std::cout << "Vulkan instance extensions:\n";

	for(const auto& extension: vk::enumerateInstanceExtensionProperties())
		std::cout << '\t' << extension.extensionName << '\n';
}

std::ostream& listQueueFamilies(std::ostream& output, std::span<const vk::QueueFamilyProperties> families)
{
	output << "Device queue families:\n";

	for(std::size_t i = 0; const auto& family: families)
	{
		output
			<< "Queue family " << i++ << ":\n"
			<< family
		;
	}

	return output;
}

std::ostream& operator<<(std::ostream& output, vk::Instance instance)
{
	for(const auto& device: instance.enumeratePhysicalDevices())
		output << device;

	return output;
}

std::ostream& operator<<(std::ostream& output, vk::PhysicalDevice physicalDevice)
{
	const auto properties = physicalDevice.getProperties();

	output
		<< "Device name: " << properties.deviceName << '\n'
		<< "Vulkan API version: "
			<< vk::versionMajor(properties.apiVersion) << '.'
			<< vk::versionMinor(properties.apiVersion) << '.'
			<< vk::versionPatch(properties.apiVersion) << '\n'
		<< "Vulkan driver version: "
			<< vk::versionMajor(properties.driverVersion) << '.'
			<< vk::versionMinor(properties.driverVersion) << '.'
			<< vk::versionPatch(properties.driverVersion) << '\n'
		<< "Device type: " << to_string(properties.deviceType) << '\n'
		<< "Max image dimension 1D: " << properties.limits.maxImageDimension1D << '\n'
		<< "Max image dimension 2D: " << properties.limits.maxImageDimension2D << '\n'
		<< "Max image dimension 3D: " << properties.limits.maxImageDimension3D << '\n'
		<< "Max compute shared memory size: " << properties.limits.maxComputeSharedMemorySize << '\n'
	;

	return listQueueFamilies(output, physicalDevice.getQueueFamilyProperties());
}

std::ostream& operator<<(std::ostream& output, vk::QueueFamilyProperties properties)
{
	output
		<< "\tQueue count: " << properties.queueCount << '\n'
		<< "\tQueue flags: " << to_string(properties.queueFlags) << '\n'
	;

	return output;
}

template vk::ArrayProxyNoTemporaries<const std::uint32_t> to_proxy(const FileBuffer& buffer);

}

