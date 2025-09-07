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

export module fdtd.services.vulkan:core;

import vulkan_hpp;
import std;

import fdtd.utils;

import :context;
import :debug_requirements;

namespace fdtd::services::vulkan
{

using namespace fdtd::utils;

export class Core
{
public:
	Core(Injector& injector);

	vk::raii::Context&        getContext();
	vk::raii::Instance&       getInstance();
	vk::raii::PhysicalDevice& getPhysicalDevice();

private:
	Context&           context;
	DebugRequirements& debugRequirements;

	vk::raii::Instance       instance       = nullptr;
	vk::raii::PhysicalDevice physicalDevice = nullptr;

	void init();

	void createInstance();

	std::vector<const char*> getRequiredLayers();
	std::vector<const char*> getRequiredExtensions();

	void checkLayers(std::span<const char* const> requiredLayers);
	void checkExtensions(std::span<const char* const> requiredExtensions);

	void createPhysicalDevice();
	vk::raii::PhysicalDevice selectPhysicalDevice();
	bool isSuitable(vk::PhysicalDevice physicalDevice);
};

}
