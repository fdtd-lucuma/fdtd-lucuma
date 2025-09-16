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

namespace fdtd::services::vulkan
{

PipelineBuilder::PipelineBuilder(Injector& injector):
	device(injector.inject<Device>()),
	shaderLoader(injector.inject<ShaderLoader>())
{
}

ComputePipeline PipelineBuilder::createComputePipeline(const ComputePipelineCreateInfo& info)
{
	return {*this, info};
}

ComputePipeline::ComputePipeline(PipelineBuilder& builder, const ComputePipelineCreateInfo& info)
{
	auto& device = builder.device.getDevice();

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

	// Create descriptor pools
	descriptorPools = info.setLayouts |
		std::views::transform([&](const auto& x)
		{
			auto poolSizes = x.bindings |
				std::views::transform([](const auto& y)
				{
					return vk::DescriptorPoolSize {
						.type            = y.descriptorType,
						.descriptorCount = y.descriptorCount,
					};
				}) |
				std::ranges::to<std::vector>()
			;

			vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo {
				.flags   = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = 1,
			};

			return device.createDescriptorPool(descriptorPoolCreateInfo);
		}) |
		std::ranges::to<std::vector>()
	;

	// Create pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo {
	};

	auto setLayouts = getDescriptorSetLayoutsUnraii();

	pipelineLayoutCreateInfo.setSetLayouts(setLayouts);

	layout = device.createPipelineLayout(pipelineLayoutCreateInfo);

	// Create pipeline
	auto shaderModule = builder.shaderLoader.createShaderModule(info.shaderPath);

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

std::span<vk::raii::DescriptorSetLayout> ComputePipeline::getDescriptorSetLayouts()
{
	return descriptorSetLayouts;
}

std::vector<vk::DescriptorSetLayout> ComputePipeline::getDescriptorSetLayoutsUnraii()
{
	return unraii(getDescriptorSetLayouts());
}

std::span<vk::raii::DescriptorPool> ComputePipeline::getDescriptorPools()
{
	return descriptorPools;
}

std::vector<vk::DescriptorPool> ComputePipeline::getDescriptorPoolsUnraii()
{
	return unraii(getDescriptorPools());
}

vk::raii::PipelineLayout& ComputePipeline::getLayout()
{
	return layout;
}

vk::raii::Pipeline& ComputePipeline::getPipeline()
{
	return pipeline;
}

}
