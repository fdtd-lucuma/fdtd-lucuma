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
	auto shaderCode = shaderLoader.createShaderModule("triangle.slang");

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

}

}
