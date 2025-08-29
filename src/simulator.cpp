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

#include <cstdlib>

module fdtd;

import std;
import magic_enum;
import fdtd.utils;

Simulator::Simulator(ArgumentParser& argumentParser):
	argumentParser(argumentParser)
{}

ArgumentParser& Simulator::getArgumentParser() const
{
	return argumentParser;
}

void listVulkanExtensions()
{
	std::cout << "Vulkan extensions:\n";

	for(const auto& extension: vk::enumerateInstanceExtensionProperties())
		std::cout << '\t' << extension.extensionName << '\n';
}

void listQueues(std::span<const vk::QueueFamilyProperties> families, std::ostream& output = std::cout)
{
	output << "Device queue families:\n";

	for(size_t i = 0; const auto& family: families)
	{
		output
			<< "Queue family " << i++ << ":\n"
			<< "\tQueue count: " << family.queueCount << '\n'
			<< "\tQueue flags: " << family.queueFlags << '\n'
		;
	}

}

void listPhysicalDevices(vk::Instance instance, std::ostream& output = std::cout)
{
	using magic_enum::iostream_operators::operator<<;

	for(const auto& device: instance.enumeratePhysicalDevices())
	{
		const auto properties = device.getProperties();

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
			<< "Device type: " << properties.deviceType << '\n'
			<< "Max image dimension 1D: " << properties.limits.maxImageDimension1D << '\n'
			<< "Max image dimension 2D: " << properties.limits.maxImageDimension2D << '\n'
			<< "Max image dimension 3D: " << properties.limits.maxImageDimension3D << '\n'
		;

		listQueues(device.getQueueFamilyProperties(), output);
	}
}

int Simulator::run()
{
	initVulkan();
	listVulkanExtensions();
	listPhysicalDevices(instance);

	return EXIT_SUCCESS;
}

void Simulator::initVulkan()
{
	constexpr vk::ApplicationInfo applicattionInfo {
		.applicationVersion = vk::makeVersion(0, 0, 0),
		.pEngineName        = "Fdtd",
		.engineVersion      = vk::makeVersion(0, 0, 0),
		.apiVersion         = vk::ApiVersion14,
	};

	vk::InstanceCreateInfo instanceCreateInfo {
		.pApplicationInfo = &applicattionInfo,
	};

	instance = context.createInstance(instanceCreateInfo);
}
