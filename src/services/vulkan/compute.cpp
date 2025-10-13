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

#include <cstddef>

module lucuma.services.vulkan;

import lucuma.services.basic;
import glm;

namespace lucuma::services::vulkan
{

using namespace lucuma::services;

Compute::Compute([[maybe_unused]] Injector& injector):
	device(injector.inject<Device>()),
	shaderLoader(injector.inject<ShaderLoader>())

{
	if(!device.getComputeInfo().has_value())
	{
		throw new std::runtime_error("Couldn't find a compute queue family.");
	}

	init();
}

vk::raii::Queue& Compute::getQueue()
{
	return queues[0];
}

vk::raii::CommandPool& Compute::getCommandPool()
{
	return commandPool;
}

void Compute::init()
{
	createQueues();
	createCommandPool();
}

void Compute::createQueues()
{
	auto info = device.getComputeInfo();

	queues =
		std::views::iota(0u, info->count) |
		std::views::transform([&](auto i){return device.getDevice().getQueue(info->index, i);}) |
		std::ranges::to<std::vector>();
}

void Compute::createCommandPool()
{
	auto info = device.getComputeInfo();

	vk::CommandPoolCreateInfo createInfo {
		.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = info->index,
	};

	commandPool = device.getDevice().createCommandPool(createInfo);
}

ComputePipeline Compute::createPipeline(const ComputePipelineCreateInfo& info)
{
	return {*this, info};
}

void Compute::submit(const vk::CommandBuffer& commandBuffer)
{
	vk::SubmitInfo submitInfo {
	};

	submitInfo.setCommandBuffers(commandBuffer);

	auto& queue = getQueue();

	queue.submit(submitInfo);
	queue.waitIdle();
}

ComputePipeline::ComputePipeline(ComputePipeline&& other):
	descriptorSetLayouts(std::exchange(other.descriptorSetLayouts, {})),
	descriptorPool(std::exchange(other.descriptorPool, nullptr)),
	commandBuffer(std::exchange(other.commandBuffer, nullptr)),
	descriptorSets(std::exchange(other.descriptorSets, {})),
	layout(std::exchange(other.layout, nullptr)),
	pipeline(std::exchange(other.pipeline, nullptr))
{}

ComputePipeline::ComputePipeline(Compute& builder, const ComputePipelineCreateInfo& info)
{
	auto& device = builder.device.getDevice();

	// Create command buffer
	// TODO: Move this outside
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo {
		.commandPool        = builder.getCommandPool(),
		.level              = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1, // TODO: Multiple frames in flight
	};

	commandBuffer = std::move(device.allocateCommandBuffers(commandBufferAllocateInfo)[0]);

	// Create descriptor set layouts
	descriptorSetLayouts = info.setLayouts |
		std::views::transform([&](auto&& x)
		{
			vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {
			};

			descriptorSetLayoutCreateInfo.setBindings(x.bindings);

			return device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
		}) |
		std::ranges::to<std::vector>()
	;

	// Create descriptor pool
	auto poolSizes = info.setLayouts |
		std::views::transform([](auto&& x) { return x.bindings; }) |
		std::views::join |
		std::views::transform([](auto&& x)
		{
			return vk::DescriptorPoolSize {
				.type            = x.descriptorType,
				.descriptorCount = x.descriptorCount,
			};
		}) |
		std::ranges::to<std::vector>()
	;

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo {
		.flags   = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = 1,
	};

	descriptorPoolCreateInfo.setPoolSizes(poolSizes);

	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);

	// Create descriptor sets
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo {
		.descriptorPool = getDescriptorPool(),
	};

	auto layouts = getDescriptorSetLayoutsUnraii();
	descriptorSetAllocateInfo.setSetLayouts(layouts);

	descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);

	// Update descriptor sets
	auto bufferInfos = info.setLayouts |
		std::views::transform([](auto&& x) { return x.buffers; }) |
		std::views::join |
		std::views::transform([](auto&& x)
		{
			return vk::DescriptorBufferInfo {
				.buffer = x.get().getBuffer(),
				.offset = 0,
				.range  = vk::WholeSize,
			};
		}) |
		std::ranges::to<std::vector>()
	;

	auto bindingDescriptors = std::views::zip(
		info.setLayouts | std::views::transform([](auto&& x) { return x.bindings; }),
		getDescriptorSets() | std::views::as_const
	);

	auto cartesianDescriptors = bindingDescriptors |
		std::views::transform([](auto&& t)
		{
			auto&& [bindings, descriptorSet] = t;

			return std::views::cartesian_product(
				bindings,
				std::views::single(std::ref(descriptorSet))
			);
		}) |
		std::views::join
	;

	auto descriptorWrite = std::views::zip(cartesianDescriptors, bufferInfos) |
		std::views::transform([](auto&& t)
		{
			auto&& [t2, bufferInfo] = t;
			auto&& [binding, descriptorSet] = t2;

			vk::WriteDescriptorSet result{
				.dstSet          = descriptorSet.get(),
				.dstBinding      = binding.binding,
				.dstArrayElement = 0,
				.descriptorCount = binding.descriptorCount,
				.descriptorType  = binding.descriptorType,
			};

			result.setBufferInfo(bufferInfo);

			return result;
		}) |
		std::ranges::to<std::vector>()
	;

	device.updateDescriptorSets(descriptorWrite, nullptr);

	// Create pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo {
	};

	auto setLayouts = getDescriptorSetLayoutsUnraii();

	pipelineLayoutCreateInfo.setSetLayouts(setLayouts);

	layout = device.createPipelineLayout(pipelineLayoutCreateInfo);

	// Workgroup size
	using wgVec = glm::vec<3, std::uint32_t>;
	static constexpr std::size_t wgDataSize = sizeof(std::uint32_t);

	wgVec wgSize = info.workgroupSize;

	std::array specializationMap {
		vk::SpecializationMapEntry{0, offsetof(wgVec, x), wgDataSize},
		vk::SpecializationMapEntry{1, offsetof(wgVec, y), wgDataSize},
		vk::SpecializationMapEntry{2, offsetof(wgVec, z), wgDataSize},
	};

	vk::SpecializationInfo specializationInfo {
	};

	specializationInfo.setData<wgVec>(wgSize);
	specializationInfo.setMapEntries(specializationMap);

	// Create pipeline
	auto shaderModule = builder.shaderLoader.createShaderModule(info.shaderPath);

	vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo {
		.stage               = vk::ShaderStageFlagBits::eCompute,
		.module              = shaderModule,
		.pName               = info.entrypoint.c_str(),
		.pSpecializationInfo = &specializationInfo,
	};

	vk::ComputePipelineCreateInfo computePipelineCreateInfo {
		.stage  = pipelineShaderStageCreateInfo,
		.layout = layout,
	};

	pipeline = device.createComputePipeline(nullptr, computePipelineCreateInfo);
}

std::span<vk::raii::DescriptorSetLayout> ComputePipeline::getDescriptorSetLayouts()
{
	return descriptorSetLayouts;
}

std::vector<vk::DescriptorSetLayout> ComputePipeline::getDescriptorSetLayoutsUnraii()
{
	return unraii(getDescriptorSetLayouts());
}

std::span<vk::raii::DescriptorSet> ComputePipeline::getDescriptorSets()
{
	return descriptorSets;
}

std::vector<vk::DescriptorSet> ComputePipeline::getDescriptorSetsUnraii()
{
	return unraii(getDescriptorSets());
}

vk::raii::DescriptorPool& ComputePipeline::getDescriptorPool()
{
	return descriptorPool;
}

vk::raii::CommandBuffer& ComputePipeline::getCommandBuffer()
{
	return commandBuffer;
}

vk::raii::PipelineLayout& ComputePipeline::getLayout()
{
	return layout;
}

vk::raii::Pipeline& ComputePipeline::getPipeline()
{
	return pipeline;
}

void ComputePipeline::bind(vk::CommandBuffer commandBuffer)
{
	constexpr auto bindPoint = vk::PipelineBindPoint::eCompute;

	commandBuffer.bindPipeline(bindPoint, getPipeline());
	commandBuffer.bindDescriptorSets(bindPoint, getLayout(), 0, getDescriptorSetsUnraii(), nullptr);
}

}
