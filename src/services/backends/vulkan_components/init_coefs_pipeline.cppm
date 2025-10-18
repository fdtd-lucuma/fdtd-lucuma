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

export module lucuma.services.backends.vulkan_components:init_coefs_pipeline;

import glm;

import lucuma.utils;
import lucuma.services.vulkan;
import vulkan_hpp;

import std;

import :utils;

namespace lucuma::services::backends::vulkan_components
{

using namespace lucuma::utils;

template <typename T>
struct InitCoefPipelineCreateInfo
{
	svec3 paddedDims;
	svec3 dims;
	T CrImp;

	vulkan::Buffer& Ch;
	vulkan::Buffer& Ce;
	vulkan::Buffer& CM;
	vulkan::Buffer& mu;

	std::filesystem::path shaderPath;
	std::string_view      entrypoint = "main";
	svec3                 workGroupSize;

	vulkan::Compute& compute;
};

template <typename T>
class InitCoefPipeline
{
private:
	struct alignas(32)
	{
		alignas(sizeof(svec4)) svec3 paddedDims;
		alignas(sizeof(svec4)) svec3 dims;
		T CrImp;
	} pushConstants;

	svec3 groupCount;

	vulkan::Buffer& Ch;
	vulkan::Buffer& Ce;
	vulkan::Buffer& CM;
	vulkan::Buffer& mu;

	vulkan::ComputePipeline pipeline;

public:
	InitCoefPipeline(const InitCoefPipelineCreateInfo<T>& createInfo):
		pushConstants({
			.paddedDims = createInfo.paddedDims,
			.dims = createInfo.dims,
			.CrImp = createInfo.CrImp,
		}),
		groupCount(createInfo.paddedDims/createInfo.workGroupSize),
		Ch(createInfo.Ch),
		Ce(createInfo.Ce),
		CM(createInfo.CM),
		mu(createInfo.mu),
		pipeline(createInfo.compute.createPipeline({
			.shaderPath = createInfo.shaderPath,
			.setLayouts = {
				{
					.bindings = simpleStorageBuffersLayout<4>(),
					.buffers = {
						createInfo.Ch,
						createInfo.Ce,
						createInfo.CM,
						createInfo.mu,
					}
				}
			},
			.workGroupSize = createInfo.workGroupSize,
			.pushConstants = vulkan::Compute::makePushConstantsLayout<typeof(pushConstants)>(),
		}))
	{ }

	void dispatch(vk::CommandBuffer commandBuffer)
	{
		pipeline.bind(commandBuffer);
		pipeline.pushConstants(commandBuffer, pushConstants);
		commandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
	}

};

struct InitCoefPipelineInfo
{
	svec3 paddedDims;
	svec3 dims;

	vulkan::Buffer&  Ch;
	vulkan::Buffer&  Ce;
	vulkan::Buffer&  CM;
	vulkan::Buffer&  mu;

	std::string_view entrypoint = "main";
};

template <typename T>
struct InitCoefPipelinesCreateInfo
{
	T Cr;
	T Imp0;

	std::filesystem::path shaderPath;
	svec3                 workGroupSize;

	vulkan::Compute& compute;

	InitCoefPipelineInfo Hx;
	InitCoefPipelineInfo Hy;
	InitCoefPipelineInfo Hz;

	InitCoefPipelineInfo Ex;
	InitCoefPipelineInfo Ey;
	InitCoefPipelineInfo Ez;

};

template <FieldType F, typename T>
InitCoefPipelineCreateInfo<T> map(const InitCoefPipelinesCreateInfo<T> createInfo, InitCoefPipelineInfo InitCoefPipelinesCreateInfo<T>::* _pipelineInfo)
{
	auto& pipelineInfo = createInfo.*_pipelineInfo;

	return {
		.paddedDims    = pipelineInfo.paddedDims,
		.dims          = pipelineInfo.dims,
		.CrImp         = crImp<F>(createInfo.Cr,createInfo.Imp0),
		.Ch            = pipelineInfo.Ch,
		.Ce            = pipelineInfo.Ce,
		.CM            = pipelineInfo.mu,
		.mu            = pipelineInfo.mu,
		.shaderPath    = createInfo.shaderPath,
		.entrypoint    = pipelineInfo.entrypoint,
		.workGroupSize = createInfo.workGroupSize,
		.compute       = createInfo.compute,
	};
}

template <typename T>
class InitCoefPipelines
{
public:
	InitCoefPipelines(InitCoefPipelinesCreateInfo<T> createInfo):
		Hx(map<FieldType::H, T>(createInfo, &InitCoefPipelinesCreateInfo<T>::Hx)),
		Hy(map<FieldType::H, T>(createInfo, &InitCoefPipelinesCreateInfo<T>::Hy)),
		Hz(map<FieldType::H, T>(createInfo, &InitCoefPipelinesCreateInfo<T>::Hz)),
		Ex(map<FieldType::E, T>(createInfo, &InitCoefPipelinesCreateInfo<T>::Ex)),
		Ey(map<FieldType::E, T>(createInfo, &InitCoefPipelinesCreateInfo<T>::Ey)),
		Ez(map<FieldType::E, T>(createInfo, &InitCoefPipelinesCreateInfo<T>::Ez))
	{ }

	void dispatch(vk::CommandBuffer commandBuffer)
	{
		Hx.dispatch(commandBuffer);
		Hy.dispatch(commandBuffer);
		Hz.dispatch(commandBuffer);

		Ex.dispatch(commandBuffer);
		Ey.dispatch(commandBuffer);
		Ez.dispatch(commandBuffer);
	}

private:
	InitCoefPipeline<T> Hx;
	InitCoefPipeline<T> Hy;
	InitCoefPipeline<T> Hz;

	InitCoefPipeline<T> Ex;
	InitCoefPipeline<T> Ey;
	InitCoefPipeline<T> Ez;
};

}
