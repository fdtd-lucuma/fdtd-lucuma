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

export module lucuma.services.backends.vulkan_components:update_h_pipeline;

import lucuma.utils;
import lucuma.services.vulkan;
import vulkan;
import std;

import :utils;

namespace lucuma::services::backends::vulkan_components
{

using namespace lucuma::utils;

template <typename T>
struct UpdateHPipelineCreateInfo
{
	svec3 paddedHDims;
	svec3 paddedEc1Dims;
	svec3 paddedEc2Dims;
	svec3 paddedMu1Dims;
	svec3 paddedMu2Dims;
	svec3 HDims;

	svec3Delta Ec1Delta;
	svec3Delta Ec2Delta;

	vulkan::Buffer& Hc;
	vulkan::Buffer& Ch;
	vulkan::Buffer& Ce;
	vulkan::Buffer& Ec1;
	vulkan::Buffer& Ec2;
	vulkan::Buffer& mu1;
	vulkan::Buffer& mu2;

	T deltaT;
	T delta1;
	T delta2;

	std::filesystem::path shaderPath;
	std::string_view      entrypoint = "main";
	svec3                 workGroupSize;

	vulkan::Compute& compute;
};

template <typename T>
class UpdateHPipeline
{
private:
	struct alignas(32)
	{
		alignas(sizeof(svec4)) svec3 paddedHDims;
		alignas(sizeof(svec4)) svec3 paddedEc1Dims;
		alignas(sizeof(svec4)) svec3 paddedEc2Dims;
		alignas(sizeof(svec4)) svec3 paddedMu1Dims;
		alignas(sizeof(svec4)) svec3 paddedMu2Dims;
		alignas(sizeof(svec4)) svec3 HDims;
		T deltaT;
		T delta1;
		T delta2;
	} pushConstants;

	svec3 groupCount;

	vulkan::ComputePipeline pipeline;

public:
	UpdateHPipeline(const UpdateHPipelineCreateInfo<T>& createInfo):
		pushConstants({
			.paddedHDims   = createInfo.paddedHDims,
			.paddedEc1Dims = createInfo.paddedEc1Dims,
			.paddedEc2Dims = createInfo.paddedEc2Dims,
			.paddedMu1Dims = createInfo.paddedMu1Dims,
			.paddedMu2Dims = createInfo.paddedMu2Dims,
			.HDims         = createInfo.HDims,
			.deltaT         = createInfo.deltaT,
			.delta1         = createInfo.delta1,
			.delta2         = createInfo.delta2,
		}),
		groupCount(workGroupCount(createInfo.paddedHDims, createInfo.workGroupSize)),
		pipeline(createInfo.compute.createPipeline({
			.shaderPath = createInfo.shaderPath,
			.setLayouts = {
				{
					.bindings = simpleStorageBuffersLayout<7>(),
					.buffers = {
						createInfo.Hc,
						createInfo.Ch,
						createInfo.Ce,
						createInfo.Ec1,
						createInfo.Ec2,
						createInfo.mu1,
						createInfo.mu2,
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
	{
		assert(groupCount*createInfo.workGroupSize == createInfo.paddedHDims);
	}

	void dispatch(vk::CommandBuffer commandBuffer)
	{
		pipeline.bind(commandBuffer);
		pipeline.pushConstants(commandBuffer, pushConstants);
		commandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
	}

};

template <typename T>
struct UpdateHPipelineInfo
{
	svec3 paddedHDims;
	svec3 paddedEc1Dims;
	svec3 paddedEc2Dims;
	svec3 paddedMu1Dims;
	svec3 paddedMu2Dims;
	svec3 HDims;

	svec3Delta Ec1Delta;
	svec3Delta Ec2Delta;

	vulkan::Buffer& Hc;
	vulkan::Buffer& Ch;
	vulkan::Buffer& Ce;
	vulkan::Buffer& Ec1;
	vulkan::Buffer& Ec2;
	vulkan::Buffer& mu1;
	vulkan::Buffer& mu2;

	T delta1;
	T delta2;

	std::string_view entrypoint = "main";
};

template <typename T>
struct UpdateHPipelinesCreateInfo
{
	std::filesystem::path shaderPath;
	svec3                 workGroupSize;

	vulkan::Compute& compute;

	T deltaT;

	UpdateHPipelineInfo<T> x;
	UpdateHPipelineInfo<T> y;
	UpdateHPipelineInfo<T> z;

};

template <typename T>
UpdateHPipelineCreateInfo<T> map(const UpdateHPipelinesCreateInfo<T>& createInfo, UpdateHPipelineInfo<T> UpdateHPipelinesCreateInfo<T>::* _pipelineInfo)
{
	auto& pipelineInfo = createInfo.*_pipelineInfo;

	return {
		.paddedHDims   = pipelineInfo.paddedHDims,
		.paddedEc1Dims = pipelineInfo.paddedEc1Dims,
		.paddedEc2Dims = pipelineInfo.paddedEc2Dims,
		.paddedMu1Dims = pipelineInfo.paddedMu1Dims,
		.paddedMu2Dims = pipelineInfo.paddedMu2Dims,
		.HDims         = pipelineInfo.HDims,
		.Ec1Delta      = pipelineInfo.Ec1Delta,
		.Ec2Delta      = pipelineInfo.Ec2Delta,
		.Hc            = pipelineInfo.Hc,
		.Ch            = pipelineInfo.Ch,
		.Ce            = pipelineInfo.Ce,
		.Ec1           = pipelineInfo.Ec1,
		.Ec2           = pipelineInfo.Ec2,
		.mu1           = pipelineInfo.mu1,
		.mu2           = pipelineInfo.mu2,
		.deltaT        = createInfo.deltaT,
		.delta1        = pipelineInfo.delta1,
		.delta2        = pipelineInfo.delta2,
		.shaderPath    = createInfo.shaderPath,
		.entrypoint    = pipelineInfo.entrypoint,
		.workGroupSize = createInfo.workGroupSize,
		.compute       = createInfo.compute,
	};
}


template <typename T>
class UpdateHPipelines
{
public:
	using create_info_t = UpdateHPipelinesCreateInfo<T>;

	UpdateHPipelines(create_info_t createInfo):
		x(map(createInfo, &create_info_t::x)),
		y(map(createInfo, &create_info_t::y)),
		z(map(createInfo, &create_info_t::z))
	{ }

	void dispatch(vk::CommandBuffer commandBuffer)
	{
		x.dispatch(commandBuffer);
		y.dispatch(commandBuffer);
		z.dispatch(commandBuffer);
	}

private:
	UpdateHPipeline<T> x;
	UpdateHPipeline<T> y;
	UpdateHPipeline<T> z;
};

}
