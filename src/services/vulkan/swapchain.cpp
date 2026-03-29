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

#include <cassert>

module lucuma.services.vulkan;

import lucuma.services.basic;
import lucuma.services.window;
import vkfw;

namespace lucuma::services::vulkan
{

using namespace lucuma::services;

Swapchain::Swapchain([[maybe_unused]] Injector& injector):
	core(injector.inject<Core>()),
	device(injector.inject<Device>()),
	settings(injector.inject<basic::Settings>()),
	glfw(injector.inject<window::Glfw>())

{
	init();
}

void Swapchain::init()
{
	createSwapchain();
	createSwapchainImageViews();
	createSyncObjects();
}

void Swapchain::createSwapchain()
{
	auto swapchainDetails = getDetails();

	auto surfaceFormat = selectDefaultSurfaceFormat(swapchainDetails);
	extent             = selectDefaultExtent(swapchainDetails);
	auto presentMode   = selectDefaultPresentMode(swapchainDetails);

	format = surfaceFormat.format;

	auto minImageCount = std::max( 3u, swapchainDetails.capabilities.minImageCount );
	minImageCount = ( swapchainDetails.capabilities.maxImageCount > 0 && minImageCount > swapchainDetails.capabilities.maxImageCount ) ? swapchainDetails.capabilities.maxImageCount : minImageCount;

	std::uint32_t imageCount = swapchainDetails.capabilities.minImageCount + 1;

	if (swapchainDetails.capabilities.maxImageCount > 0 && imageCount > swapchainDetails.capabilities.maxImageCount)
		imageCount = swapchainDetails.capabilities.maxImageCount;

	vk::SwapchainCreateInfoKHR createInfo {
		.flags            = vk::SwapchainCreateFlagsKHR(),
		.surface          = *core.getSurface(),
		.minImageCount    = minImageCount,
		.imageFormat      = format,
		.imageColorSpace  = surfaceFormat.colorSpace,
		.imageExtent      = extent,
		.imageArrayLayers = 1,
		.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
		.preTransform     = swapchainDetails.capabilities.currentTransform,
		.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode      = presentMode,
		.clipped          = true,
		.oldSwapchain     = nullptr,
	};

	assert(device.getGraphicsInfo().has_value());
	assert(device.getPresentInfo().has_value());

	std::array<std::uint32_t, 2> queueFamilyIndices = {
		device.getGraphicsInfo()->index,
		device.getPresentInfo()->index,
	};

	if(queueFamilyIndices[0] != queueFamilyIndices[1])
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.setQueueFamilyIndices(queueFamilyIndices);
	}
	else
	{
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.setQueueFamilyIndices({});
	}

	swapchain       = vk::raii::SwapchainKHR(device.getDevice(), createInfo);
	swapchainImages = swapchain.getImages();
}

void Swapchain::createSwapchainImageViews()
{
	swapchainImageViews.clear();
	swapchainImageViews.reserve(swapchainImages.size());

	vk::ImageViewCreateInfo createInfo {
		.viewType = vk::ImageViewType::e2D,
		.format   = format,

		.subresourceRange = {
			.aspectMask     = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel   = 0,
			.levelCount     = 1,
			.baseArrayLayer = 0,
			.layerCount     = 1,
		}

	};

	for(const auto& image: swapchainImages) {
		createInfo.image = image;
		swapchainImageViews.emplace_back(device.getDevice(), createInfo);
	}

}

SwapchainDetails Swapchain::getDetails() const
{
	const auto& physicalDevice = core.getPhysicalDevice();
	const auto& surface = core.getSurface();

	return SwapchainDetails{
		.capabilities          = physicalDevice.getSurfaceCapabilitiesKHR(surface),
		.availableFormats      = physicalDevice.getSurfaceFormatsKHR(surface),
		.availablePresentModes = physicalDevice.getSurfacePresentModesKHR(surface),
	};
}

vk::SurfaceFormatKHR Swapchain::selectDefaultSurfaceFormat(const SwapchainDetails& details) const
{
	for(const auto& availableFormat: details.availableFormats)
	{
		// TODO: Set preferred from settings
		if(availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			return availableFormat;
	}

	return details.availableFormats[0];
}

vk::PresentModeKHR Swapchain::selectDefaultPresentMode(const SwapchainDetails& details) const
{
	for(const auto& availablePresentMode: details.availablePresentModes)
	{
		if(availablePresentMode == vk::PresentModeKHR::eFifo) //Vsync
			return availablePresentMode;
	}

	return details.availablePresentModes[0];
}

vk::Extent2D Swapchain::selectDefaultExtent(const SwapchainDetails& details) const
{
	if(details.capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
		return details.capabilities.currentExtent;

	auto [width, height] = glfw.getFramebufferSize();

	return vk::Extent2D {
		.width  = std::clamp<std::uint32_t>(width, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width),
		.height = std::clamp<std::uint32_t>(height, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height),
	};
}

vk::Format Swapchain::getFormat() const
{
	return format;
}

vk::Extent2D Swapchain::getExtent() const
{
	return extent;
}

vk::Viewport Swapchain::getCurrentViewport() const
{
	return vk::Viewport {
		.x        = 0.0f,
		.y        = 0.0f,
		.width    = (float)getExtent().width,
		.height   = (float)getExtent().height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
}

vk::Rect2D Swapchain::getCurrentScissor() const
{
	return vk::Rect2D {
		.offset = {
			.x = 0,
			.y = 0,
		},
		.extent = getExtent(),
	};

}

std::span<const vk::Image> Swapchain::getImages() const
{
	return swapchainImages;
}

std::span<const vk::raii::ImageView> Swapchain::getImageViews() const
{
	return swapchainImageViews;
}

vk::raii::SwapchainKHR& Swapchain::getSwapchain()
{
	return swapchain;
}

const vk::raii::SwapchainKHR& Swapchain::getSwapchain() const
{
	return swapchain;
}

void Swapchain::advanceFrame()
{
	semaphoreIndex = (semaphoreIndex + 1) % presentCompleteSemaphores.size();
}

vk::raii::Semaphore& Swapchain::getCurrentPresentCompleteSemaphore()
{
	return presentCompleteSemaphores[semaphoreIndex];
}

vk::raii::Semaphore& Swapchain::getCurrentRenderFinishedSemaphore()
{
	return renderFinishedSemaphores[semaphoreIndex];
}

void Swapchain::createSyncObjects()
{
	presentCompleteSemaphores.clear();
	renderFinishedSemaphores.clear();

	for(std::size_t i = 0; i < swapchainImages.size(); i++)
	{
		presentCompleteSemaphores.emplace_back(device.getDevice().createSemaphore({}));
		renderFinishedSemaphores.emplace_back(device.getDevice().createSemaphore({}));
	}

}

void Swapchain::recreate()
{
	glfw.waitUntilMaximixed();
	device.getDevice().waitIdle();

	cleanupSwapchain();

	createSwapchain();
	createSwapchainImageViews();
}

void Swapchain::cleanupSwapchain()
{
	swapchainImageViews.clear();
	swapchain = nullptr;
}

}
