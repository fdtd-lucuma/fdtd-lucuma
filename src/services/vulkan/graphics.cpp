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

#include <vulkan/vulkan.hpp>
#include <tracy/TracyVulkan.hpp>

module lucuma.services.vulkan;

import lucuma.services.window;
import lucuma.services.basic;
import lucuma.legacy_headers.entt;
import lucuma.events.vulkan;

import vkfw;

namespace lucuma::services::vulkan
{

using namespace lucuma::services;

Graphics::Graphics([[maybe_unused]] Injector& injector):
	dispatcher(injector.inject<entt::dispatcher>()),
	registry(injector.inject<entt::registry>()),
	device(injector.inject<Device>()),
	swapchain(injector.inject<Swapchain>()),
	settings(injector.inject<basic::Settings>()),
	glfw(injector.inject<window::Glfw>())

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

	createCommandBuffers();

	createSyncObjects();
	createTracyContexts();

	glfw.getWindow().callbacks()->on_framebuffer_resize =
		[this](const vkfw::Window&, std::size_t, std::size_t)
		{
			framebufferResized = true;
		}
	;
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

	{
		auto* ctx = getCurrentVkCtx();
		TracyVkNamedZone(ctx, __zone, *commandBuffer, "Graphics", settings.tracy());

		recordCommandBufferInner(imageIndex);

		if(ctx != nullptr && currentFrame % 65 == 0)
		{
			TracyVkCollect(ctx, *commandBuffer);
		}
	}

	commandBuffer.end();

}

void Graphics::recordCommandBufferInner(std::uint32_t imageIndex)
{
	auto& commandBuffer = getCurrentCommandBuffer();

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

	dispatcher.trigger(events::vulkan::Draw{commandBuffer});
	dispatcher.trigger(events::vulkan::GuiDraw{commandBuffer});

	commandBuffer.endRendering();

	transitionImageOptimal2PresentSrc(imageIndex);

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
	inFlightFences.clear();

	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		inFlightFences.emplace_back(device.getDevice().createFence({.flags = vk::FenceCreateFlagBits::eSignaled}));
	}

}

void Graphics::createTracyContexts()
{
	if(!settings.tracy())
		return;

	tracyContexts.clear();

	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		const auto& physdev = *device.getPhysicalDevice();
		const auto& dev     = *device.getDevice();
		const auto& queue   = *getGraphicsQueue();
		const auto& cmdbuf  = commandBuffers[i];

		tracy::VkCtx* ptr = TracyVkContext(physdev, dev, queue, *cmdbuf);

		tracyContexts.emplace_back(ptr);
	}
}

vk::Result Graphics::acquireNextImage()
{
	auto [result, _imageIndex] = swapchain
		.getSwapchain()
		.acquireNextImage(std::numeric_limits<std::uint64_t>::max(), swapchain.getCurrentPresentCompleteSemaphore(), nullptr);

	currentImageIndex = _imageIndex;
	return result;
}

void Graphics::draw()
{
	waitFence();
	auto result = acquireNextImage();

	if(result == vk::Result::eErrorOutOfDateKHR)
	{
		swapchain.recreate();
		return;
	}

	if(result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	device.getDevice().resetFences(*getCurrentInFlightFence());
	recordCommandBuffer(currentImageIndex);

	vk::PipelineStageFlags waitDestinationStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo;

	submitInfo
		.setWaitDstStageMask(waitDestinationStageMask)
		.setWaitSemaphores(*swapchain.getCurrentPresentCompleteSemaphore())
		.setCommandBuffers(*getCurrentCommandBuffer())
		.setSignalSemaphores(*swapchain.getCurrentRenderFinishedSemaphore())
	;

	getGraphicsQueue().submit(submitInfo, getCurrentInFlightFence());

	result = present();

	if(result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized)
	{
		swapchain.recreate();
		framebufferResized = false;
	}
	else if(result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	advanceFrame();
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
		.setWaitSemaphores(*swapchain.getCurrentRenderFinishedSemaphore())
		.setSwapchains(*swapchain.getSwapchain())
		.setImageIndices(currentImageIndex)
	;

	return getPresentQueue().presentKHR(presentInfo);
}

Graphics::~Graphics()
{
	device.waitIdle();

	for(auto ptr: tracyContexts)
	{
		TracyVkDestroy(ptr);
	}
}

vk::raii::CommandBuffer& Graphics::getCurrentCommandBuffer()
{
	return commandBuffers[currentFrameMod];
}

vk::raii::Fence& Graphics::getCurrentInFlightFence()
{
	return inFlightFences[currentFrameMod];
}

tracy::VkCtx* Graphics::getCurrentVkCtx()
{
	if(tracyContexts.empty())
		return nullptr;

	return tracyContexts[currentFrameMod];
}

void Graphics::advanceFrame()
{
	currentFrameMod = ++currentFrame % MAX_FRAMES_IN_FLIGHT;
	swapchain.advanceFrame();
}

}
