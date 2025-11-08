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

vk::raii::Queue& Graphics::getGraphicsQueue()
{
	return graphicsQueues[0];
}

vk::raii::CommandPool& Graphics::getGraphicsCommandPool()
{
	return graphicsCommandPool;
}

vk::raii::Queue& Graphics::getPresentQueue()
{
	return graphicsQueues[0];
}

vk::raii::CommandPool& Graphics::getPresentCommandPool()
{
	return graphicsCommandPool;
}

void Graphics::init()
{
	createQueues();
	createCommandPools();

	// TODO: Move this
	createGraphicsPipeline();
	createCommandBuffers();

	createSyncObjects();
}

std::vector<vk::raii::Queue> Graphics::createQueuesCommon(const QueueFamilyInfo& info)
{
	return
		std::views::iota(0u, info.count) |
		std::views::transform([&](auto i){return device.getDevice().getQueue(info.index, i);}) |
		std::ranges::to<std::vector>();
}

void Graphics::createQueues()
{
	graphicsQueues = createQueuesCommon(device.getGraphicsInfo().value());
	presentQueues = createQueuesCommon(device.getPresentInfo().value());
}

void Graphics::createCommandPools()
{
	graphicsCommandPool = createCommandPool(device.getGraphicsInfo().value());
	presentCommandPool  = createCommandPool(device.getPresentInfo().value());
}

vk::raii::CommandPool Graphics::createCommandPool(const QueueFamilyInfo& info)
{
	vk::CommandPoolCreateInfo createInfo {
		.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = info.index,
	};

	return device.getDevice().createCommandPool(createInfo);
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

void Graphics::createCommandBuffers()
{
	vk::CommandBufferAllocateInfo allocInfo {
		.commandPool        = graphicsCommandPool,
		.level              = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT,
	};

	commandBuffers = device.getDevice().allocateCommandBuffers(allocInfo);
}

void Graphics::recordCommandBuffer(std::uint32_t imageIndex)
{
	auto& commandBuffer = getCurrentCommandBuffer();

	commandBuffer.reset();

	commandBuffer.begin({});

	transitionImageAny2Optimal(imageIndex);

	vk::ClearValue clearColor = vk::ClearColorValue(0,0,0,1);
	vk::RenderingAttachmentInfo attachmentInfo {
		.imageView   = swapchain.getImageViews()[imageIndex],
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp      = vk::AttachmentLoadOp::eClear,
		.storeOp     = vk::AttachmentStoreOp::eStore,
		.clearValue  = clearColor,
	};

	vk::RenderingInfo renderingInfo {
		.renderArea = {
			.offset = {0,0},
			.extent = swapchain.getExtent(),
		},
		.layerCount = 1,
	};

	renderingInfo.setColorAttachments(attachmentInfo);

	commandBuffer.beginRendering(renderingInfo);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
	commandBuffer.setViewport(0, swapchain.getCurrentViewport());
	commandBuffer.setScissor(0, swapchain.getCurrentScissor());

	commandBuffer.draw(3, 1, 0, 0);

	commandBuffer.endRendering();

	transitionImageOptimal2PresentSrc(imageIndex);

	commandBuffer.end();

}

void Graphics::transition_image_layout(const TransitionImageLayoutInfo& input)
{
	auto& commandBuffer = getCurrentCommandBuffer();

	vk::ImageMemoryBarrier2 barrier {
		.srcStageMask        = input.srcStageMask,
		.srcAccessMask       = input.srcAccessMask,
		.dstStageMask        = input.dstStageMask,
		.dstAccessMask       = input.dstAccessMask,
		.oldLayout           = input.oldLayout,
		.newLayout           = input.newLayout,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image               = swapchain.getImages()[input.imageIndex],
		.subresourceRange = {
			.aspectMask     = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel   = 0,
			.levelCount     = 1,
			.baseArrayLayer = 0,
			.layerCount     = 1,
		},
	};

	vk::DependencyInfo dependencyInfo;
	dependencyInfo.setImageMemoryBarriers(barrier);

	commandBuffer.pipelineBarrier2(dependencyInfo);
}

void Graphics::transitionImageAny2Optimal(std::uint32_t imageIndex)
{
	transition_image_layout({
		.imageIndex    = imageIndex,
		.oldLayout     = vk::ImageLayout::eUndefined,
		.newLayout     = vk::ImageLayout::eColorAttachmentOptimal,
		.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
		.srcStageMask  = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		.dstStageMask  = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
	});
}

void Graphics::transitionImageOptimal2PresentSrc(std::uint32_t imageIndex)
{
	transition_image_layout({
		.imageIndex    = imageIndex,
		.oldLayout     = vk::ImageLayout::eColorAttachmentOptimal,
		.newLayout     = vk::ImageLayout::ePresentSrcKHR,
		.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
		.srcStageMask  = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		.dstStageMask  = vk::PipelineStageFlagBits2::eBottomOfPipe,
	});
}

void Graphics::createSyncObjects()
{
	presentCompleteSemaphores.clear();
	renderFinishedSemaphores.clear();
	inFlightFences.clear();

	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		presentCompleteSemaphores.emplace_back(device.getDevice().createSemaphore({}));
		renderFinishedSemaphores.emplace_back(device.getDevice().createSemaphore({}));

		inFlightFences.emplace_back(device.getDevice().createFence({.flags = vk::FenceCreateFlagBits::eSignaled}));
	}


}

vk::Result Graphics::acquireNextImage()
{
	auto [result, _imageIndex] = swapchain
		.getSwapchain()
		.acquireNextImage(std::numeric_limits<std::uint64_t>::max(), getCurrentPresentCompleteSemaphore(), nullptr);

	currentImageIndex = _imageIndex;
	return result;
}

void Graphics::draw()
{
	device.getDevice().resetFences(*getCurrentInFlightFence());
	recordCommandBuffer(currentImageIndex);

	vk::PipelineStageFlags waitDestinationStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo;

	submitInfo
		.setWaitDstStageMask(waitDestinationStageMask)
		.setWaitSemaphores(*getCurrentPresentCompleteSemaphore())
		.setCommandBuffers(*getCurrentCommandBuffer())
		.setSignalSemaphores(*getCurrentRenderFinishedSemaphore())
	;

	getGraphicsQueue().submit(submitInfo, getCurrentInFlightFence());
}

void Graphics::waitFence()
{
	while(vk::Result::eTimeout == device
		.getDevice()
		.waitForFences(*getCurrentInFlightFence(), true, std::numeric_limits<std::uint64_t>::max())
	);
}

vk::Result Graphics::present()
{
	vk::PresentInfoKHR presentInfo {
		.pResults = nullptr,
	};

	presentInfo
		.setWaitSemaphores(*getCurrentRenderFinishedSemaphore())
		.setSwapchains(*swapchain.getSwapchain())
		.setImageIndices(currentImageIndex)
	;

	auto result = getPresentQueue().presentKHR(presentInfo);

	advanceFrame();

	return result;
}

Graphics::~Graphics()
{
	device.getDevice().waitIdle();
}

vk::raii::CommandBuffer& Graphics::getCurrentCommandBuffer()
{
	return commandBuffers[currentFrame];
}

vk::raii::Semaphore& Graphics::getCurrentPresentCompleteSemaphore()
{
	return presentCompleteSemaphores[currentFrame];
}

vk::raii::Semaphore& Graphics::getCurrentRenderFinishedSemaphore()
{
	return renderFinishedSemaphores[currentFrame];
}

vk::raii::Fence& Graphics::getCurrentInFlightFence()
{
	return inFlightFences[currentFrame];
}

void Graphics::advanceFrame()
{
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

}
