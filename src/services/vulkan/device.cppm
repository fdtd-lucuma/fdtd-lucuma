// Una GUI para fdtd
// Copyright © 2025-2026 Otreblan
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

export module lucuma.services.vulkan:device;

import vulkan_hpp;
import std;

import lucuma.utils;
import lucuma.services.basic;

namespace lucuma::services::vulkan
{

using namespace lucuma::utils;
using namespace lucuma::services;

struct QueueFamilyInfo
{
	std::uint32_t index = 0;
	std::uint32_t count = 0;

	std::vector<float> priorities = std::vector<float>(count, 1.0f);

	auto operator<=>(QueueFamilyInfo const &) const = default;
};

class Core;

export class Device
{
public:
	Device(Injector& injector);

	vk::raii::PhysicalDevice& getPhysicalDevice();
	const vk::raii::PhysicalDevice& getPhysicalDevice() const;

	vk::raii::Device& getDevice();
	const vk::raii::Device& getDevice() const;

	const std::optional<QueueFamilyInfo>& getGraphicsInfo() const;
	const std::optional<QueueFamilyInfo>& getComputeInfo() const;
	const std::optional<QueueFamilyInfo>& getPresentInfo() const;

	void waitIdle() const;

private:
	Core&            core;
	basic::Settings& settings;

	vk::raii::Device device = nullptr;

	std::optional<QueueFamilyInfo> computeInfo  = std::nullopt;
	std::optional<QueueFamilyInfo> graphicsInfo = std::nullopt;
	std::optional<QueueFamilyInfo> presentInfo = std::nullopt;

	std::vector<const char*> getRequiredExtensions();

	void init();

	vk::DeviceQueueCreateInfo getComputeQueueCreateInfo(std::span<const vk::QueueFamilyProperties> properties);
	vk::DeviceQueueCreateInfo getGraphicsQueueCreateInfo(std::span<const vk::QueueFamilyProperties> properties);

	std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos();
	void createDevice();

	QueueFamilyInfo selectQueueFamilyCommon(std::span<const vk::QueueFamilyProperties> properties, vk::QueueFlags queueFlags);

	QueueFamilyInfo selectComputeQueueFamily(std::span<const vk::QueueFamilyProperties> properties);
	QueueFamilyInfo selectGraphicsQueueFamily(std::span<const vk::QueueFamilyProperties> properties);
};

}
