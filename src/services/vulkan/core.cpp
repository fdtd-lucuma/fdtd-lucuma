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

#include <ranges>

module fdtd.services.vulkan;

namespace fdtd::services::vulkan
{

Core::Core([[maybe_unused]] Injector& injector):
	context(injector.inject<Context>()),
	debugRequirements(injector.inject<DebugRequirements>())
{
	init();
}

vk::raii::Context& Core::getContext()
{
	return context.getContext();
}

vk::raii::Instance& Core::getInstance()
{
	return instance;
}

vk::raii::PhysicalDevice& Core::getPhysicalDevice()
{
	return physicalDevice;
}

void Core::init()
{
	createInstance();
	createPhysicalDevice();
}

void Core::createInstance()
{
	constexpr vk::ApplicationInfo applicattionInfo {
		.applicationVersion = vk::makeVersion(0, 0, 0),
		.pEngineName        = "Fdtd",
		.engineVersion      = vk::makeVersion(0, 0, 0),
		.apiVersion         = vk::ApiVersion14,
	};

	auto layers     = getRequiredLayers();
	auto extensions = getRequiredExtensions();

	vk::InstanceCreateInfo instanceCreateInfo {
		.pApplicationInfo = &applicattionInfo,
	};

	instanceCreateInfo
		.setPEnabledLayerNames(layers)
		.setPEnabledExtensionNames(extensions);

	instance = getContext().createInstance(instanceCreateInfo);
}

std::vector<const char*> Core::getRequiredLayers()
{
	auto result = std::views::concat(
		debugRequirements.getRequiredLayers()
		) | std::ranges::to<std::vector>();

	checkLayers(result);

	return result;
}

std::vector<const char*> Core::getRequiredExtensions()
{

	auto result = std::views::concat(
		debugRequirements.getRequiredExtensions()
		// TODO: glfw
		) | std::ranges::to<std::vector>();

	checkExtensions(result);

	return result;
}

bool noAnyLayerInProperties(
	std::span<const char* const> requiredLayers,
	std::span<const vk::LayerProperties> layerProperties
)
{
	return std::ranges::any_of(requiredLayers,
		[&](const auto& requiredLayer)
		{
			return std::ranges::none_of(layerProperties,
				[&](const auto& layerProperty)
				{
					return strcmp(layerProperty.layerName, requiredLayer) == 0;
				}
			);
		}
	);
}

bool noExtensionInProperties(
	const char* const requiredExtension,
	std::span<const vk::ExtensionProperties> extensionProperties
)
{
	return std::ranges::none_of(extensionProperties,
		[&](const auto& extensionProperty)
		{
			return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
		}
	);
}

void Core::checkLayers(std::span<const char* const> requiredLayers)
{
	const auto layerProperties = getContext().enumerateInstanceLayerProperties();

	if(noAnyLayerInProperties(requiredLayers, layerProperties))
	{
		std::cerr << "Required layers:\n";

		for(const auto& layer: requiredLayers)
			std::cerr << '\t' << layer << '\n';

		throw std::runtime_error("One or more required layers are not supported");
	}

}

void Core::checkExtensions(std::span<const char* const> requiredExtensions)
{
	const auto extensionProperties = getContext().enumerateInstanceExtensionProperties();

	for(const auto& extension: requiredExtensions)
	{
		if(noExtensionInProperties(extension, extensionProperties))
		{
			throw std::runtime_error("Required extension not supported: " + std::string(extension));
		}
	}

}

void Core::createPhysicalDevice()
{
	physicalDevice = selectPhysicalDevice();
}

vk::raii::PhysicalDevice Core::selectPhysicalDevice()
{
	auto filter = [this](const auto& x){return isSuitable(x);};

	// TODO: Force from command line arguments
	for(const auto& device:
		getInstance().enumeratePhysicalDevices() |
		std::views::filter(filter)
	)
	{
		return device;
	}

	throw std::runtime_error("Failed to find a  compatible GPU");
}

bool Core::isSuitable(vk::PhysicalDevice physicalDevice)
{
	auto properties = physicalDevice.getProperties();
	//auto features   = physicalDevice.getFeatures();

	if(properties.apiVersion < vk::ApiVersion14)
		return false;

	// TODO: Check for extensions

	return true;
}

}
