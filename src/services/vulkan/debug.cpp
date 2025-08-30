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

#include <vulkan/vk_platform.h>

module fdtd.services.vulkan;

import std;

VulkanDebug::VulkanDebug([[maybe_unused]] Injector& injector):
	vulkanCore(injector.inject<VulkanCore>()),
	vulkanDebugRequirements(injector.inject<VulkanDebugRequirements>())
{
	init();
}

std::vector<const char*> VulkanDebug::getRequiredLayers()
{
	return vulkanDebugRequirements.getRequiredLayers();
}

std::vector<const char*> VulkanDebug::getRequiredExtensions()
{
	return vulkanDebugRequirements.getRequiredExtensions();
}

void VulkanDebug::init()
{
	if constexpr(VulkanDebugRequirements::enableValidationLayers)
		setupDebugMessenger();
}

void VulkanDebug::setupDebugMessenger()
{
	using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using enum vk::DebugUtilsMessageTypeFlagBitsEXT;

	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags {
		eVerbose |
		eWarning |
		eError
	};

	vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags {
		eGeneral |
		ePerformance |
		eValidation
	};

	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
		.messageSeverity = severityFlags,
		.messageType = messageTypeFlags,
		.pfnUserCallback = &debugCallback,
		.pUserData = this,
	};

	debugMessenger = vulkanCore.getInstance().createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanDebug::debugCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT      severity,
	vk::DebugUtilsMessageTypeFlagsEXT             type,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*                                         pUserData
)
{
	return ((VulkanDebug*)pUserData)->debugCallback(severity, type, pCallbackData);
}

vk::Bool32 VulkanDebug::debugCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT      severity,
	vk::DebugUtilsMessageTypeFlagsEXT             type,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData
)
{
	std::cerr
		<< "Validation layer: type " << to_string(type) << '\n'
		<< "\tmsg: " << pCallbackData->pMessage << '\n'
		<< "\tseverity: " << to_string(severity) << '\n'
	;

	return vk::False;
}
