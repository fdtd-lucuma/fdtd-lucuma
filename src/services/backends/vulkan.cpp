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

module lucuma.services.backends;

import lucuma.utils;
import lucuma.services.vulkan;
import lucuma.legacy_headers.entt;
import std;
import vulkan_hpp;
import vk_mem_alloc_hpp;


import :vulkan;

namespace lucuma::services::backends
{

VulkanBase::VulkanBase([[maybe_unused]]Injector& injector):
	vulkanAllocator(injector.inject<vulkan::Allocator>()),
	vulkanCompute(injector.inject<vulkan::Compute>()),
	vulkanAll(injector.inject<vulkan::All>()),
	settings(injector.inject<basic::Settings>()),
	registry(injector.inject<entt::registry>())
{
	init();
}

void VulkanBase::init()
{
	commandBuffer = vulkanCompute.createSimpleCommandBuffer();
}

vulkan::CommandRecorder VulkanBase::createCommandRecorder()
{
	return vulkanCompute.createCommandRecorder(commandBuffer);
}

template class Vulkan<Precision::f16>;
template class Vulkan<Precision::f32>;
template class Vulkan<Precision::f64>;

}
