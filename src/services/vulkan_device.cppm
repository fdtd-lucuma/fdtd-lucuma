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

export module fdtd.services:vulkan_device;

import :vulkan_core;

export import fdtd.utils;
export import vulkan_hpp;

export class VulkanDevice
{
public:
	VulkanDevice(Injector& injector);

	vk::raii::PhysicalDevice& getPhysicalDevice();
	vk::raii::Device&         getDevice();

private:
	VulkanCore& vulkanCore;

	vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::Device         device         = nullptr;

	void init();

	void createDevices();

	vk::raii::PhysicalDevice selectPhysicalDevice();
	bool isSuitable(vk::PhysicalDevice physicalDevice);

};
