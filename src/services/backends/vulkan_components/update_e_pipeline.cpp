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

module lucuma.services.backends.vulkan_components;

namespace lucuma::services::backends::vulkan_components
{

UpdateEPipeline::UpdateEPipeline(const UpdateEPipelineCreateInfo& createInfo):
	pushConstants({
		.paddedEDims   = createInfo.paddedEDims,
		.paddedHc1Dims = createInfo.paddedHc1Dims,
		.paddedHc2Dims = createInfo.paddedHc2Dims,
		.dims          = createInfo.dims,
		.start         = createInfo.start,
	}),
	groupCount(workGroupCount(createInfo.paddedEDims, createInfo.workGroupSize)),
	pipeline(createInfo.compute.createPipeline({
		.shaderPath = createInfo.shaderPath,
		.setLayouts = {
			{
				.bindings = simpleStorageBuffersLayout<5>(),
				.buffers = {
					createInfo.Ec,
					createInfo.Ce,
					createInfo.Ch,
					createInfo.Hc1,
					createInfo.Hc2,
				}
			}
		},
		.pushConstants = vulkan::Compute::makePushConstantsLayout<typeof(pushConstants)>(),
		.specializationConstants = workgroupSizeWithDeltas(
			createInfo.workGroupSize,
			createInfo.Hc1Delta,
			createInfo.Hc2Delta
		),
	}))
{ }

void UpdateEPipeline::dispatch(vk::CommandBuffer commandBuffer)
{
	pipeline.bind(commandBuffer);
	pipeline.pushConstants(commandBuffer, pushConstants);
	commandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
}

UpdateEPipelineCreateInfo map(const UpdateEPipelinesCreateInfo createInfo, UpdateEPipelineInfo UpdateEPipelinesCreateInfo::* _pipelineInfo)
{
	auto& pipelineInfo = createInfo.*_pipelineInfo;

	return {
		.paddedEDims   = pipelineInfo.paddedEDims,
		.paddedHc1Dims = pipelineInfo.paddedHc1Dims,
		.paddedHc2Dims = pipelineInfo.paddedHc2Dims,
		.dims          = createInfo.dims,
		.start         = pipelineInfo.start,
		.Hc1Delta      = pipelineInfo.Hc1Delta,
		.Hc2Delta      = pipelineInfo.Hc2Delta,
		.Ec            = pipelineInfo.Ec,
		.Ce            = pipelineInfo.Ce,
		.Ch            = pipelineInfo.Ch,
		.Hc1           = pipelineInfo.Hc1,
		.Hc2           = pipelineInfo.Hc2,
		.shaderPath    = createInfo.shaderPath,
		.entrypoint    = pipelineInfo.entrypoint,
		.workGroupSize = createInfo.workGroupSize,
		.compute       = createInfo.compute,
	};
}

UpdateEPipelines::UpdateEPipelines(create_info_t createInfo):
	x(map(createInfo, &create_info_t::x)),
	y(map(createInfo, &create_info_t::y)),
	z(map(createInfo, &create_info_t::z))
{ }

void UpdateEPipelines::dispatch(vk::CommandBuffer commandBuffer)
{
	x.dispatch(commandBuffer);
	y.dispatch(commandBuffer);
	z.dispatch(commandBuffer);
}

}
