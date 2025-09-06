// Una GUI para fdtd
// Copyright © 2025 Otreblan
//
// fdtd-vulkan is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// fdtd-vulkan is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fdtd-vulkan.  If not, see <http://www.gnu.org/licenses/>.

module;

module fdtd.services.vulkan;

VulkanPipelineBuilder::VulkanPipelineBuilder(Injector& injector):
	vulkanDevice(injector.inject<VulkanDevice>()),
	vulkanShaderLoader(injector.inject<VulkanShaderLoader>())
{
}

VulkanComputePipelineData VulkanPipelineBuilder::createComputePipeline(const VulkanComputePipelineInfo& info)
{
	return {*this, info};
}

VulkanComputePipelineData::VulkanComputePipelineData(VulkanPipelineBuilder& builder, const VulkanComputePipelineInfo& info)
{
	auto& device = builder.vulkanDevice.getDevice();

	// Create descriptor set layouts
	descriptorSetLayouts = info.setLayouts |
		std::views::transform([&](const auto& x)
		{
			vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {
			};

			descriptorSetLayoutCreateInfo.setBindings(x.bindings);

			return device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
		}) |
		std::ranges::to<std::vector>()
	;

	// Create pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo {
	};

	auto setLayouts = descriptorSetLayouts |
		std::views::transform([](const auto& x){return *x;}) |
		std::ranges::to<std::vector>();

	pipelineLayoutCreateInfo.setSetLayouts(setLayouts);

	layout = device.createPipelineLayout(pipelineLayoutCreateInfo);

	// Create pipeline
	auto shaderModule = builder.vulkanShaderLoader.createShaderModule(info.shaderPath);

	vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo {
		.stage  = vk::ShaderStageFlagBits::eCompute,
		.module = shaderModule,
		.pName  = info.entrypoint.c_str(),
	};

	vk::ComputePipelineCreateInfo computePipelineCreateInfo {
		.stage  = pipelineShaderStageCreateInfo,
		.layout = layout,
	};

	pipeline = device.createComputePipeline(nullptr, computePipelineCreateInfo);
}

