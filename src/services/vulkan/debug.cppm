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

export module fdtd.services.vulkan:debug;

import vulkan_hpp;
import std;

import fdtd.utils;

namespace fdtd::services::vulkan
{

using namespace fdtd::utils;

class Core;
class DebugRequirements;

export class Debug
{
public:
	Debug(Injector& injector);

	std::vector<const char*> getRequiredLayers();
	std::vector<const char*> getRequiredExtensions();

private:
	Core&              core;
	DebugRequirements& debugRequirements;

	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

	void init();

	void setupDebugMessenger();

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT      severity,
		vk::DebugUtilsMessageTypeFlagsEXT             type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void*                                         pUserData
	);

	vk::Bool32 debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT      severity,
		vk::DebugUtilsMessageTypeFlagsEXT             type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData
	);

};

}
