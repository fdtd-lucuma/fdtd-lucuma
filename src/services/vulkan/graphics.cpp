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

module lucuma.services.vulkan;

import lucuma.services.basic;

namespace lucuma::services::vulkan
{

using namespace lucuma::services;

Graphics::Graphics([[maybe_unused]] Injector& injector):
	device(injector.inject<Device>()),
	shaderLoader(injector.inject<ShaderLoader>()),
	swapchain(injector.inject<Swapchain>())

{
	if(!device.getGraphicsInfo().has_value())
	{
		throw new std::runtime_error("Couldn't find a graphics queue family.");
	}

	init();
}

vk::raii::Queue& Graphics::getQueue()
{
	return queues[0];
}

vk::raii::CommandPool& Graphics::getCommandPool()
{
	return commandPool;
}

void Graphics::init()
{
	createQueues();
	createCommandPool();
	createGraphicsPipeline();
}

void Graphics::createQueues()
{
	auto info = device.getGraphicsInfo();

	queues =
		std::views::iota(0u, info->count) |
		std::views::transform([&](auto i){return device.getDevice().getQueue(info->index, i);}) |
		std::ranges::to<std::vector>();
}

void Graphics::createCommandPool()
{
	auto info = device.getGraphicsInfo();

	vk::CommandPoolCreateInfo createInfo {
		.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = info->index,
	};

	commandPool = device.getDevice().createCommandPool(createInfo);
}

void Graphics::createGraphicsPipeline()
{
	auto shaderCode = shaderLoader.createShaderModule("triangle.spv");

	vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo {
		.stage  = vk::ShaderStageFlagBits::eVertex,
		.module = shaderCode,
		.pName  = "vertMain",
	};

	vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo {
		.stage  = vk::ShaderStageFlagBits::eFragment,
		.module = shaderCode,
		.pName  = "fragMain",
	};

	vk::PipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageCreateInfo,
		fragShaderStageCreateInfo,
	};

	std::vector dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
	};

	vk::PipelineDynamicStateCreateInfo dynamicState;
	dynamicState.setDynamicStates(dynamicStates);

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly {
		.topology = vk::PrimitiveTopology::eTriangleList,
	};

	vk::PipelineViewportStateCreateInfo viewportState {
		.viewportCount = 1,
		.scissorCount  = 1,
	};

	vk::PipelineRasterizationStateCreateInfo rasterizer {
		.depthClampEnable        = false,
		.rasterizerDiscardEnable = false,
		.polygonMode             = vk::PolygonMode::eFill,
		.cullMode                = vk::CullModeFlagBits::eBack,
		.frontFace               = vk::FrontFace::eClockwise,
		.depthBiasEnable         = false,
		.depthBiasSlopeFactor    = 1.0f,
		.lineWidth               = 1.0f,
	};

	// TODO: Get from settings
	vk::PipelineMultisampleStateCreateInfo multisampling {
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable  = false,
	};

	vk::PipelineColorBlendAttachmentState colorBlendAttachment {
		.blendEnable = false,
		.colorWriteMask =
			vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA,
	};

	vk::PipelineColorBlendStateCreateInfo colorBlending {
		.logicOpEnable = false,
		.logicOp       = vk::LogicOp::eCopy,
	};

	colorBlending.setAttachments(colorBlendAttachment);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo {
		.setLayoutCount         = 0,
		.pushConstantRangeCount = 0,
	};

	pipelineLayout = device.getDevice().createPipelineLayout(pipelineLayoutInfo);
}

}
