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

module fdtd.services;

import fdtd.utils;
import std;
import vulkan_hpp;
import vk_mem_alloc_hpp;

namespace fdtd::services
{

Compute::Compute([[maybe_unused]]Injector& injector):
	vulkanPipelineBuilder(injector.inject<vulkan::PipelineBuilder>()),
	vulkanAllocator(injector.inject<vulkan::Allocator>())
{ }

void Compute::compute()
{
	auto pipeline = vulkanPipelineBuilder.createComputePipeline({
		.shaderPath = "./share/shaders/hello_world.spv",
		.setLayouts = {
			{
				.bindings = {
					vk::DescriptorSetLayoutBinding {
						.binding        = 0,
						.descriptorType = vk::DescriptorType::eStorageBuffer,
						.stageFlags     = vk::ShaderStageFlagBits::eCompute,
					},
					vk::DescriptorSetLayoutBinding {
						.binding        = 1,
						.descriptorType = vk::DescriptorType::eStorageBuffer,
						.stageFlags     = vk::ShaderStageFlagBits::eCompute,
					},
					vk::DescriptorSetLayoutBinding {
						.binding        = 2,
						.descriptorType = vk::DescriptorType::eStorageBuffer,
						.stageFlags     = vk::ShaderStageFlagBits::eCompute,
					},
				}
			}
		}
	});

	auto buffer = vulkanAllocator.allocate(
		sizeof(float)*1,
		vk::BufferUsageFlagBits::eStorageBuffer,
		vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
	);

	//TODO: Stuff

	vulkanAllocator.flush(buffer);

}

}
