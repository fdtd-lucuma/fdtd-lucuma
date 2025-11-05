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

#include <cassert>

module lucuma.services.vulkan;

import lucuma.services.basic;

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

	auto [width, height] = glfw.getWindow().getFramebufferSize();

	return vk::Extent2D {
		.width  = std::clamp<std::uint32_t>(width, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width),
		.height = std::clamp<std::uint32_t>(height, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height),
	};
}

}
