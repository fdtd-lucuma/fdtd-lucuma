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

export module fdtd.services.vulkan:device;

import vulkan_hpp;
import std;

import fdtd.utils;

namespace fdtd::services::vulkan
{

using namespace fdtd::utils;

struct QueueFamilyInfo
{
	std::uint32_t index = 0;
	std::uint32_t count = 0;

	std::vector<float> priorities = std::vector<float>(count, 1.0f);
};

class Core;

export class Device
{
public:
	Device(Injector& injector);

	vk::raii::PhysicalDevice& getPhysicalDevice();
	vk::raii::Device&         getDevice();
	vk::raii::Queue&          getComputeQueue();

private:
	Core& core;

	vk::raii::Device device = nullptr;

	QueueFamilyInfo              computeQueueInfo;
	std::vector<vk::raii::Queue> computeQueues;

	std::vector<const char*> getRequiredLayers();
	std::vector<const char*> getRequiredExtensions();

	void init();

	vk::DeviceQueueCreateInfo getComputeQueueCreateInfo(std::span<const vk::QueueFamilyProperties> properties);

	std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos();
	void createDevice();

	QueueFamilyInfo selectComputeQueueFamily(std::span<const vk::QueueFamilyProperties> properties);

	std::vector<vk::raii::Queue> createQueues(const QueueFamilyInfo& info);
};

}

// Explicit template instantiations for faster compilation
namespace fdtd::utils
{
using namespace fdtd::services::vulkan;

template Device& Injector::inject<Device>();

}
