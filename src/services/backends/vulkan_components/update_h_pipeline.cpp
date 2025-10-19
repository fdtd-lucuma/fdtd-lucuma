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

UpdateHPipeline::UpdateHPipeline(const UpdateHPipelineCreateInfo& createInfo):
	pushConstants({
		.paddedHDims   = createInfo.paddedHDims,
		.paddedEc1Dims = createInfo.paddedEc1Dims,
		.paddedEc2Dims = createInfo.paddedEc2Dims,
		.HDims         = createInfo.HDims,
	}),
	groupCount(createInfo.paddedHDims/createInfo.workGroupSize),
	pipeline(createInfo.compute.createPipeline({
		.shaderPath = createInfo.shaderPath,
		.setLayouts = {
			{
				.bindings = simpleStorageBuffersLayout<5>(),
				.buffers = {
					createInfo.Hc,
					createInfo.Ch,
					createInfo.Ce,
					createInfo.Ec1,
					createInfo.Ec2,
				}
			}
		},
		.pushConstants = vulkan::Compute::makePushConstantsLayout<typeof(pushConstants)>(),
		.specializationConstants = workgroupSizeWithDeltas(
			createInfo.workGroupSize,
			createInfo.Ec1Delta,
			createInfo.Ec2Delta
		),
	}))
{ }

void UpdateHPipeline::dispatch(vk::CommandBuffer commandBuffer)
{
	pipeline.bind(commandBuffer);
	pipeline.pushConstants(commandBuffer, pushConstants);
	commandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
}

UpdateHPipelineCreateInfo map(const UpdateHPipelinesCreateInfo createInfo, UpdateHPipelineInfo UpdateHPipelinesCreateInfo::* _pipelineInfo)
{
	auto& pipelineInfo = createInfo.*_pipelineInfo;

	return {
		.paddedHDims   = pipelineInfo.paddedHDims,
		.paddedEc1Dims = pipelineInfo.paddedEc1Dims,
		.paddedEc2Dims = pipelineInfo.paddedEc2Dims,
		.HDims         = pipelineInfo.HDims,
		.Ec1Delta      = pipelineInfo.Ec1Delta,
		.Ec2Delta      = pipelineInfo.Ec2Delta,
		.Hc            = pipelineInfo.Hc,
		.Ch            = pipelineInfo.Ch,
		.Ce            = pipelineInfo.Ce,
		.Ec1           = pipelineInfo.Ec1,
		.Ec2           = pipelineInfo.Ec2,
		.shaderPath    = createInfo.shaderPath,
		.entrypoint    = pipelineInfo.entrypoint,
		.workGroupSize = createInfo.workGroupSize,
		.compute       = createInfo.compute,
	};
}

UpdateHPipelines::UpdateHPipelines(create_info_t createInfo):
	x(map(createInfo, &create_info_t::x)),
	y(map(createInfo, &create_info_t::y)),
	z(map(createInfo, &create_info_t::z))
{ }

void UpdateHPipelines::dispatch(vk::CommandBuffer commandBuffer)
{
	x.dispatch(commandBuffer);
	y.dispatch(commandBuffer);
	z.dispatch(commandBuffer);
}

}
