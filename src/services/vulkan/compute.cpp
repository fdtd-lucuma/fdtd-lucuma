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

#include <cstddef>
#include <vulkan/vulkan.hpp>
#include <tracy/TracyVulkan.hpp>

module lucuma.services.vulkan;

import lucuma.services.basic;
import lucuma.legacy_headers.glm;

namespace lucuma::services::vulkan
{

using namespace lucuma::services;

Compute::Compute([[maybe_unused]] Injector& injector):
	device(injector.inject<Device>()),
	shaderLoader(injector.inject<ShaderLoader>()),
	performance(injector.inject<Performance>()),
	settings(injector.inject<basic::Settings>())

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

SimpleCommandBuffer Compute::createSimpleCommandBuffer()
{
	return {*this};
}

CommandRecorder Compute::createCommandRecorder(vk::CommandBuffer commandBuffer)
{
	CommandRecorderCreateInfo createInfo {
		.commandBuffer = commandBuffer,
		.compute       = *this,
	};

	return {createInfo};
}

void Compute::submit(const vk::CommandBuffer& commandBuffer)
{
	vk::SubmitInfo submitInfo {
	};

	submitInfo.setCommandBuffers(commandBuffer);

	auto& queue = getQueue();

	queue.submit(submitInfo);
	queue.waitIdle(); //TODO: Proper fence
}

std::tuple<svec3, std::uint64_t> limits(const vk::PhysicalDeviceLimits limits)
{
	return {{limits.maxComputeWorkGroupSize[0], limits.maxComputeWorkGroupSize[1], limits.maxComputeWorkGroupSize[2]}, limits.maxComputeWorkGroupInvocations};
}

bool fits(svec3 workGroupSize, svec3 maxWorkgroupSize, svec3 size, std::uint64_t maxInvocations)
{
	return glm::all(glm::lessThanEqual(workGroupSize, maxWorkgroupSize)) &&
		glm::all(glm::lessThanEqual(workGroupSize, size)) &&
		glm::compMul(workGroupSize) <= maxInvocations;
}

svec3 Compute::getWorkgroupSize(svec3 size) const
{
	svec3 result(1);

	auto [wgSizes, wgInvocations] = limits(device.getPhysicalDevice().getProperties().limits);

	for(svec3 x = result; fits(x, wgSizes, size, wgInvocations); x *= 2)
	{
		result = x;
	}

	return result;
}

ComputePipeline::ComputePipeline(ComputePipeline&& other):
	descriptorSetLayouts(std::exchange(other.descriptorSetLayouts, {})),
	descriptorPool(std::exchange(other.descriptorPool, nullptr)),
	descriptorSets(std::exchange(other.descriptorSets, {})),
	layout(std::exchange(other.layout, nullptr)),
	pipeline(std::exchange(other.pipeline, nullptr))
{}

ComputePipeline::ComputePipeline(Compute& builder, const ComputePipelineCreateInfo& info)
{
	auto& device = builder.device.getDevice();

	initDescriptorSets(device, info);
	initLayout(device, info);

	// Create pipeline
	auto shaderModule = builder.shaderLoader.createShaderModule(info.shaderPath);
	auto specializationInfo = info.specializationConstants.getInfo();

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

void ComputePipeline::initDescriptorSets(vk::raii::Device& device, const ComputePipelineCreateInfo& info)
{
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
		getDescriptorSets()
	);

	auto cartesianDescriptors = bindingDescriptors |
		std::views::transform([](auto&& t)
		{
			auto&& [bindings, descriptorSet] = t;


#ifdef __APPLE__
			std::vector<std::pair<vk::DescriptorSetLayoutBinding, std::reference_wrapper<const vk::raii::DescriptorSet>>> result;

			for(auto&& x: bindings)
				result.emplace_back(x, std::ref(descriptorSet));

			return result;
#else
			return std::views::cartesian_product(
				bindings,
				std::views::single(std::ref(descriptorSet))
			);
#endif
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
}
void ComputePipeline::initLayout(vk::raii::Device& device, const ComputePipelineCreateInfo& info)
{
	// Create pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo {
	};

	auto setLayouts = getDescriptorSetLayoutsUnraii();

	pipelineLayoutCreateInfo.setSetLayouts(setLayouts);
	pipelineLayoutCreateInfo.setPushConstantRanges(info.pushConstants);

	layout = device.createPipelineLayout(pipelineLayoutCreateInfo);

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

SimpleCommandBuffer::SimpleCommandBuffer(SimpleCommandBuffer&& other):
	commandBuffer(std::exchange(other.commandBuffer, nullptr)),
	ctx(std::exchange(other.ctx, nullptr))
{}

SimpleCommandBuffer::SimpleCommandBuffer(Compute& compute)
{
	auto& device = compute.device.getDevice();

	// Create command buffer
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo {
		.commandPool        = compute.getCommandPool(),
		.level              = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1, // TODO: Multiple frames in flight
	};

	commandBuffer = std::move(device.allocateCommandBuffers(commandBufferAllocateInfo)[0]);

	if(compute.settings.tracy())
	{
		const auto& physdev = *compute.device.getPhysicalDevice();
		const auto& dev     = *device;
		const auto& queue   = *compute.getQueue();
		const auto& cmdbuf  = *commandBuffer;

		ctx = TracyVkContext(physdev, dev, queue, cmdbuf);
	}
}

vk::raii::CommandBuffer& SimpleCommandBuffer::getCommandBuffer()
{
	return commandBuffer;
}

SimpleCommandBuffer::operator vk::CommandBuffer()
{
	return getCommandBuffer();
}

vk::raii::CommandBuffer* SimpleCommandBuffer::operator ->()
{
	return &getCommandBuffer();
}

SimpleCommandBuffer::~SimpleCommandBuffer()
{
	if(ctx == nullptr)
		return;

	TracyVkDestroy(ctx);
}

void SimpleCommandBuffer::tracyCollect()
{
	if(ctx == nullptr)
		return;

	TracyVkCollect(ctx, *commandBuffer);
}

tracy::VkCtx* SimpleCommandBuffer::getCtx()
{
	return ctx;
}

CommandRecorder::CommandRecorder(const CommandRecorderCreateInfo& createInfo):
	commandBuffer(createInfo.commandBuffer),
	compute(&createInfo.compute)
{
	init();
}

CommandRecorder::CommandRecorder(CommandRecorder&& other):
	commandBuffer(std::exchange(other.commandBuffer, {})),
	compute(std::exchange(other.compute, nullptr))
{
}

void CommandRecorder::init()
{
	// TODO: Find out how to mae this work with tracy
	//if(compute->tracy())
	//{
	//	compute->startPerformanceRecording(commandBuffer);
	//	return;
	//}

	commandBuffer.reset();

	vk::CommandBufferBeginInfo beginInfo {
	};

	commandBuffer.begin(beginInfo);
}

vk::CommandBuffer& CommandRecorder::getCommandBuffer()
{
	return commandBuffer;
}

CommandRecorder::operator vk::CommandBuffer&()
{
	return getCommandBuffer();
}

vk::CommandBuffer* CommandRecorder::operator ->()
{
	return &getCommandBuffer();
}

CommandRecorder::~CommandRecorder()
{
	if(!commandBuffer || compute == nullptr)
		return;


	// TODO: Find out how to mae this work with tracy
	//if(compute->tracy())
	//{
	//	compute->submitPerformancePasses(commandBuffer);
	//	return;
	//}

	commandBuffer.end();
	compute->submit(getCommandBuffer()); // TODO: Fence?
}

bool Compute::tracy()
{
	return settings.tracy();
}

void Compute::startPerformanceRecording(vk::CommandBuffer buf)
{
	performance.startRecording(buf);
}

void Compute::submitPerformancePasses(vk::CommandBuffer buf)
{
	performance.submitPasses(getQueue(), buf);
}

}
