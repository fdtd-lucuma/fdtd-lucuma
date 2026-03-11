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

module lucuma.services.backends;

import lucuma.utils;
import lucuma.services.vulkan;
import lucuma.legacy_headers.entt;
import std;
import vulkan_hpp;
import vk_mem_alloc;


import :vulkan;

namespace lucuma::services::backends
{

VulkanBase::VulkanBase([[maybe_unused]]Injector& injector):
	vulkanAllocator(injector.inject<vulkan::Allocator>()),
	vulkanCompute(injector.inject<vulkan::Compute>()),
	vulkanAll(injector.inject<vulkan::All>()),
	settings(injector.inject<basic::Settings>()),
	fileReader(injector.inject<basic::FileReader>()),
	registry(injector.inject<entt::registry>()),
	dispatcher(injector.inject<entt::dispatcher>()),
	commandBuffer(vulkanCompute.createSimpleCommandBuffer())
{
}

VulkanBase::VulkanBase(VulkanBase&& other):
	vulkanAllocator(other.vulkanAllocator),
	vulkanCompute(other.vulkanCompute),
	vulkanAll(other.vulkanAll),
	settings(other.settings),
	fileReader(other.fileReader),
	registry(other.registry),
	dispatcher(other.dispatcher),
	commandBuffer(std::exchange(other.commandBuffer, {}))
{
}

void VulkanBase::init()
{
	// TODO: Check why this = operator calls the copy constructor
	//commandBuffer = vulkanCompute.createSimpleCommandBuffer();
}

vulkan::CommandRecorder VulkanBase::createCommandRecorder()
{
	return vulkanCompute.createCommandRecorder(commandBuffer);
}

using enum vk::PipelineStageFlagBits2;
using enum vk::AccessFlagBits2;

constexpr vk::AccessFlags2 writeAccessMask(vk::PipelineStageFlags2 stage)
{
	vk::AccessFlags2 result;

	if(stage & eComputeShader)
		result |= eShaderWrite;
	if(stage & eHost)
		result |= eHostWrite;

	return result;
}

constexpr vk::AccessFlags2 readAccessMask(vk::PipelineStageFlags2 stage)
{
	vk::AccessFlags2 result;

	if(stage & eComputeShader)
		result |= eShaderRead;
	if(stage & eHost)
		result |= eHostRead;

	return result;
}

void writeReadBarrier(vk::CommandBuffer commandBuffer, vk::PipelineStageFlags2 src, vk::PipelineStageFlags2 dst)
{
	vk::MemoryBarrier2 memoryBarrier {
		.srcStageMask  = src,
		.srcAccessMask = writeAccessMask(src),
		.dstStageMask  = dst,
		.dstAccessMask = readAccessMask(dst),
	};

	vk::DependencyInfo dependencyInfo {
	};

	dependencyInfo.setMemoryBarriers(memoryBarrier);

	commandBuffer.pipelineBarrier2(dependencyInfo);
}

void VulkanBase::computeComputeBarrier(vk::CommandBuffer commandBuffer)
{
	writeReadBarrier(commandBuffer, eComputeShader, eComputeShader);
}

void VulkanBase::computeCpuBarrier(vk::CommandBuffer commandBuffer)
{
	writeReadBarrier(commandBuffer, eComputeShader, eHost);
}

void VulkanBase::initCommon(const components::FdtdDataCreateInfo& createInfo, entt::entity id)
{
	if(settings.debug())
		std::println("Starting Vulkan simulation #{} with:\n{}", id, createInfo);
}

template class Vulkan<Precision::f16>;
template class Vulkan<Precision::f32>;
template class Vulkan<Precision::f64>;

}
