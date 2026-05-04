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

export module lucuma.services.backends.vulkan_components:update_e_pipeline;

import lucuma.utils;
import lucuma.services.vulkan;
import vulkan_hpp;
import std;

import :utils;

namespace lucuma::services::backends::vulkan_components
{

using namespace lucuma::utils;

template <typename T>
struct UpdateEPipelineCreateInfo
{
	svec3 paddedEDims;
	svec3 paddedHc1Dims;
	svec3 paddedHc2Dims;
	svec3 dims;
	svec3 start;

	svec3Delta Hc1Delta;
	svec3Delta Hc2Delta;

	vulkan::Buffer& Ec;
	vulkan::Buffer& Ce;
	vulkan::Buffer& Ch;
	vulkan::Buffer& Hc1;
	vulkan::Buffer& Hc2;

	T deltaT;
	T delta1;
	T delta2;

	std::filesystem::path shaderPath;
	std::string_view      entrypoint = "main";
	svec3                 workGroupSize;

	vulkan::Compute& compute;
};

template <typename T>
class UpdateEPipeline
{
private:
	struct alignas(32)
	{
		alignas(sizeof(svec4)) svec3 paddedEDims;
		alignas(sizeof(svec4)) svec3 paddedHc1Dims;
		alignas(sizeof(svec4)) svec3 paddedHc2Dims;
		alignas(sizeof(svec4)) svec3 dims;
		alignas(sizeof(svec4)) svec3 start;
		T deltaT;
		T delta1;
		T delta2;
	} pushConstants;

	svec3 groupCount;

	vulkan::ComputePipeline pipeline;

public:
	UpdateEPipeline(const UpdateEPipelineCreateInfo<T>& createInfo):
		pushConstants({
			.paddedEDims   = createInfo.paddedEDims,
			.paddedHc1Dims = createInfo.paddedHc1Dims,
			.paddedHc2Dims = createInfo.paddedHc2Dims,
			.dims          = createInfo.dims,
			.start         = createInfo.start,
			.deltaT = createInfo.deltaT,
			.delta1 = createInfo.delta1,
			.delta2 = createInfo.delta2,
		}),
		groupCount(workGroupCount(createInfo.paddedEDims, createInfo.workGroupSize)),
		pipeline(createInfo.compute.createPipeline({
			.shaderPath = createInfo.shaderPath,
			.setLayouts = std::array{
				vulkan::ComputePipelineCreateInfo::setLayout {
					.bindings = simpleStorageBuffersLayout<7>(),
					.buffers = std::array<std::reference_wrapper<const vulkan::Buffer>, 5>{
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
	{
		assert(groupCount*createInfo.workGroupSize == createInfo.paddedEDims);
	}


	void dispatch(vk::CommandBuffer commandBuffer)
	{
		pipeline.bind(commandBuffer);
		pipeline.pushConstants(commandBuffer, pushConstants);
		commandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
	}


};

template <typename T>
struct UpdateEPipelineInfo
{
	svec3 paddedEDims;
	svec3 paddedHc1Dims;
	svec3 paddedHc2Dims;
	svec3 start;

	svec3Delta Hc1Delta;
	svec3Delta Hc2Delta;

	vulkan::Buffer& Ec;
	vulkan::Buffer& Ce;
	vulkan::Buffer& Ch;
	vulkan::Buffer& Hc1;
	vulkan::Buffer& Hc2;

	T delta1;
	T delta2;

	std::string_view entrypoint = "main";
};

template <typename T>
struct UpdateEPipelinesCreateInfo
{
	std::filesystem::path shaderPath;
	svec3                 workGroupSize;
	svec3                 dims;

	vulkan::Compute& compute;

	T deltaT;

	UpdateEPipelineInfo<T> x;
	UpdateEPipelineInfo<T> y;
	UpdateEPipelineInfo<T> z;

};


template <typename T>
UpdateEPipelineCreateInfo<T> map(const UpdateEPipelinesCreateInfo<T>& createInfo, UpdateEPipelineInfo<T> UpdateEPipelinesCreateInfo<T>::* _pipelineInfo)
{
	auto& pipelineInfo = createInfo.*_pipelineInfo;

	return {
		.paddedEDims    = pipelineInfo.paddedEDims,
		.paddedHc1Dims  = pipelineInfo.paddedHc1Dims,
		.paddedHc2Dims  = pipelineInfo.paddedHc2Dims,
		.dims           = createInfo.dims,
		.start          = pipelineInfo.start,
		.Hc1Delta       = pipelineInfo.Hc1Delta,
		.Hc2Delta       = pipelineInfo.Hc2Delta,
		.Ec             = pipelineInfo.Ec,
		.Ce             = pipelineInfo.Ce,
		.Ch             = pipelineInfo.Ch,
		.Hc1            = pipelineInfo.Hc1,
		.Hc2            = pipelineInfo.Hc2,
		.deltaT         = createInfo.deltaT,
		.delta1         = pipelineInfo.delta1,
		.delta2         = pipelineInfo.delta2,
		.shaderPath     = createInfo.shaderPath,
		.entrypoint     = pipelineInfo.entrypoint,
		.workGroupSize  = createInfo.workGroupSize,
		.compute        = createInfo.compute,
	};
}

template <typename T>
class UpdateEPipelines
{
public:
	using create_info_t = UpdateEPipelinesCreateInfo<T>;

	UpdateEPipelines(create_info_t createInfo):
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
	UpdateEPipeline<T> x;
	UpdateEPipeline<T> y;
	UpdateEPipeline<T> z;
};

}
