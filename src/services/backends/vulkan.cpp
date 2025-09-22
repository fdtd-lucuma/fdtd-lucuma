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
import std;
import vulkan_hpp;
import vk_mem_alloc_hpp;

namespace lucuma::services::backends
{

Vulkan::Vulkan([[maybe_unused]]Injector& injector):
	vulkanAllocator(injector.inject<vulkan::Allocator>()),
	vulkanCompute(injector.inject<vulkan::Compute>()),
	vulkanAll(injector.inject<vulkan::All>()),
	settings(injector.inject<basic::Settings>())
{ }

void Vulkan::init()
{
	helloWorld();
	//TODO
}

bool Vulkan::step()
{
	//TODO
	return false;
}

void Vulkan::saveFiles()
{
	//TODO
}

Vulkan::HelloWorldData Vulkan::createHelloWorld(std::size_t n)
{
	HelloWorldData result;

	result.aBuffer = vulkanAllocator.allocate(
		sizeof(float)*n,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped
	);

	result.bBuffer = vulkanAllocator.allocate(
		sizeof(float)*n,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped
	);

	result.cBuffer = vulkanAllocator.allocate(
		sizeof(float)*n,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
		vma::AllocationCreateFlagBits::eHostAccessRandom | vma::AllocationCreateFlagBits::eMapped
	);

	result.pipeline = vulkanCompute.createPipeline({
		.shaderPath = "hello_world_float.spv",
		.setLayouts = {
			{
				.bindings = {
					vk::DescriptorSetLayoutBinding {
						.binding         = 0,
						.descriptorType  = vk::DescriptorType::eStorageBuffer,
						.descriptorCount = 1,
						.stageFlags      = vk::ShaderStageFlagBits::eCompute,
					},
					vk::DescriptorSetLayoutBinding {
						.binding         = 1,
						.descriptorType  = vk::DescriptorType::eStorageBuffer,
						.descriptorCount = 1,
						.stageFlags      = vk::ShaderStageFlagBits::eCompute,
					},
					vk::DescriptorSetLayoutBinding {
						.binding         = 2,
						.descriptorType  = vk::DescriptorType::eStorageBuffer,
						.descriptorCount = 1,
						.stageFlags      = vk::ShaderStageFlagBits::eCompute,
					},
				},
				.buffers = {
					result.aBuffer,
					result.bBuffer,
					result.cBuffer,
				}
			}
		},
	});

	return result;

}

void Vulkan::helloWorld()
{
	auto generator = std::views::iota(0, 10);

	std::vector<float> a{std::from_range, generator};
	std::vector<float> b{std::from_range, generator | std::views::transform([](auto&& x){return x*x;})};

	auto pipeline = createHelloWorld(a.size());

	pipeline.aBuffer.setData<float>(a);
	pipeline.bBuffer.setData<float>(b);

	auto& commandBuffer = pipeline.pipeline.getCommandBuffer();

	vk::CommandBufferBeginInfo beginInfo{};

	commandBuffer.begin(beginInfo);

	pipeline.pipeline.bind(commandBuffer);
	commandBuffer.dispatch(a.size(), 1, 1);

	commandBuffer.end();

	vulkanCompute.submit(commandBuffer);

	auto c = pipeline.cBuffer.getData<float>().subspan(0, a.size());

	std::println("{} + {} = {}", a, b, c);
}


}
