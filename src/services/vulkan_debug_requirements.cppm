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

export module fdtd.services:vulkan_debug_requirements;

export import fdtd.utils;

import std;

export class VulkanDebugRequirements
{
public:
	VulkanDebugRequirements(Injector& injector);

	std::vector<const char*> getRequiredLayers();
	std::vector<const char*> getRequiredExtensions();

#ifdef NDEBUG
	constexpr static bool enableValidationLayers = false;
#else
	constexpr static bool enableValidationLayers = true;
#endif

private:
	constexpr static std::array<const char*, 1> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
};
