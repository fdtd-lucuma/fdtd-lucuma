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

import lucuma.services.window;
import lucuma.services.basic;
import lucuma.legacy_headers.entt;
import lucuma.events.vulkan;

import vkfw;

namespace lucuma::services::vulkan
{

using namespace lucuma::services;

TriangleDemo::TriangleDemo([[maybe_unused]] Injector& injector):
	dispatcher(injector.inject<entt::dispatcher>()),
	registry(injector.inject<entt::registry>()),
	device(injector.inject<Device>()),
	shaderLoader(injector.inject<ShaderLoader>()),
	swapchain(injector.inject<Swapchain>())

{
	init();
}

void TriangleDemo::init()
{
	createGraphicsPipeline();

	dispatcher.sink<events::vulkan::Draw>().connect<&TriangleDemo::onDraw>(*this);
}

void TriangleDemo::createGraphicsPipeline()
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

	vk::StructureChain chain {
		vk::GraphicsPipelineCreateInfo {
			.pVertexInputState   = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState      = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState   = &multisampling,
			.pColorBlendState    = &colorBlending,
			.pDynamicState       = &dynamicState,
			.layout              = pipelineLayout,
			.renderPass          = nullptr,
		},
		vk::PipelineRenderingCreateInfo{},
	};

	auto& [pipelineInfo, pipelineRenderingCreateInfo] = chain;

	const auto swapchainFormat = swapchain.getFormat();

	pipelineInfo.setStages(shaderStages);
	pipelineRenderingCreateInfo.setColorAttachmentFormats(swapchainFormat);

	pipeline = device.getDevice().createGraphicsPipeline(nullptr, pipelineInfo);
}

void TriangleDemo::onDraw(const events::vulkan::Draw& event)
{
	event.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
	event.commandBuffer.setViewport(0, swapchain.getCurrentViewport());
	event.commandBuffer.setScissor(0, swapchain.getCurrentScissor());

	event.commandBuffer.draw(3, 1, 0, 0);
}

TriangleDemo::~TriangleDemo()
{
	dispatcher.sink<events::vulkan::Draw>().disconnect<&TriangleDemo::onDraw>(*this);
}

}
