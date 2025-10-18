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

std::ostream& listMemoryProperties(std::ostream& output, std::span<const vk::MemoryType> memoryTypes)
{
	output << "Device memory types:\n";

	for(std::size_t i = 0; const auto& memoryType: memoryTypes)
	{
		output
			<< "Memory type " << i++ << ":\n"
			<< "\tProperties: " << to_string(memoryType.propertyFlags) << '\n'
			<< "\tHeap index: " << memoryType.heapIndex << '\n'
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
	const auto chain = physicalDevice.getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceVulkan12Properties>();
	const auto memoryProperties = physicalDevice.getMemoryProperties();

	const auto& properties = chain.get<vk::PhysicalDeviceProperties2>().properties;
	const auto& v12Properties = chain.get<vk::PhysicalDeviceVulkan12Properties>();

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
		<< "Vulkan driver name: "
			<< v12Properties.driverName << '\n'
		<< "Vulkan driver info: "
			<< v12Properties.driverInfo << '\n'
		<< "Device type: " << to_string(properties.deviceType) << '\n'
		<< "Max image dimension 1D: " << properties.limits.maxImageDimension1D << '\n'
		<< "Max image dimension 2D: " << properties.limits.maxImageDimension2D << '\n'
		<< "Max image dimension 3D: " << properties.limits.maxImageDimension3D << '\n'
		<< "Max workgroup size X: " << properties.limits.maxComputeWorkGroupSize[0] << '\n'
		<< "Max workgroup size Y: " << properties.limits.maxComputeWorkGroupSize[1] << '\n'
		<< "Max workgroup size Z: " << properties.limits.maxComputeWorkGroupSize[2] << '\n'
		<< "Max total workgroup size (X*Y*Z): " << properties.limits.maxComputeWorkGroupInvocations << '\n'
		<< "Max compute shared memory size: " << properties.limits.maxComputeSharedMemorySize << '\n'
	;

	listMemoryProperties(output, {memoryProperties.memoryTypes.data(), memoryProperties.memoryTypeCount});
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

