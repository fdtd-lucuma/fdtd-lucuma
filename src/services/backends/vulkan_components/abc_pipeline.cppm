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

export module lucuma.services.backends.vulkan_components:abc_pipeline;

import glm;

import lucuma.utils;
import lucuma.utils.vulkan;
import lucuma.services.vulkan;
import vulkan_hpp;

import std;

import :utils;

namespace lucuma::services::backends::vulkan_components
{

using namespace lucuma::utils;
using namespace lucuma::utils::vulkan;

template <typename T>
struct AbcSlicedPipelineCreateInfo
{

	svec3 paddedDims;
	svec3 paddedEcDims;
	svec2 paddedecDims;
	svec3 dims;
	T Cr;

	Dim dim;

	vulkan::Buffer& mu;
	vulkan::Buffer& eps;

	vulkan::Buffer& Ec;
	vulkan::Buffer& ec;

	std::size_t sliceIndex;
	std::ptrdiff_t sliceDelta;

	std::filesystem::path shaderPath;
	std::string_view      entrypoint = "main";
	svec3                 workGroupSize;

	vulkan::Compute& compute;
};

template <typename T>
class AbcSlicedPipeline
{
private:
	struct alignas(32)
	{
		alignas(sizeof(svec4)) svec3 paddedDims;
		alignas(sizeof(svec4)) svec3 paddedEcDims;
		alignas(sizeof(svec4)) svec3 dims;
		alignas(sizeof(svec2)) svec2 paddedecDims;
		T Cr;
	} pushConstants;

	svec3 groupCount;

	vulkan::ComputePipeline pipeline;
public:
	AbcSlicedPipeline(const AbcSlicedPipelineCreateInfo<T>& createInfo):
		pushConstants({
			.paddedDims   = createInfo.paddedDims,
			.paddedEcDims = createInfo.paddedEcDims,
			.dims         = createInfo.dims,
			.paddedecDims = createInfo.paddedecDims,
			.Cr           = createInfo.Cr,
		}),
		groupCount(workGroupCount(slice(createInfo.paddedEcDims, createInfo.dim), createInfo.workGroupSize)),
		pipeline(createInfo.compute.createPipeline({
			.shaderPath = createInfo.shaderPath,
			.setLayouts = std::array{
				vulkan::ComputePipelineCreateInfo::setLayout {
					.bindings = simpleStorageBuffersLayout<4>(),
					.buffers = std::array<std::reference_wrapper<const vulkan::Buffer>, 4>{
						createInfo.mu,
						createInfo.eps,
						createInfo.Ec,
						createInfo.ec,
					}
				}
			},
			.pushConstants = vulkan::Compute::makePushConstantsLayout<typeof(pushConstants)>(),
			.specializationConstants = workgroupSizeWithDimAndSlices(
				createInfo.workGroupSize,
				createInfo.dim,
				createInfo.sliceIndex,
				createInfo.sliceDelta
			),
		}))
	{
		assert(countOnes(groupCount) >= 1);
		assert(groupCount*createInfo.workGroupSize == slice(createInfo.paddedEcDims, createInfo.dim));
	}

	void dispatch(vk::CommandBuffer commandBuffer)
	{
		pipeline.bind(commandBuffer);
		pipeline.pushConstants(commandBuffer, pushConstants);
		commandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
	}
};

struct AbcSlicedInfo
{
	svec3 paddedEcDims;
	svec2 paddedecDims;

	vulkan::Buffer& Ec;
	vulkan::Buffer& ec;
};

template <typename T>
struct AbcDoublePipelinesCreateInfo
{

	svec3 paddedDims;
	svec3 dims;
	T Cr;

	Dim dim;

	vulkan::Buffer& mu;
	vulkan::Buffer& eps;

	AbcSlicedInfo slice1;
	AbcSlicedInfo slice2;

	std::size_t sliceIndex;
	std::ptrdiff_t sliceDelta;

	std::filesystem::path shaderPath;
	std::string_view      entrypoint = "main";
	svec3                 workGroupSize;

	vulkan::Compute& compute;
};

template <typename T>
AbcSlicedPipelineCreateInfo<T> map(const AbcDoublePipelinesCreateInfo<T> createInfo, AbcSlicedInfo AbcDoublePipelinesCreateInfo<T>::* _pipelineInfo)
{
	auto& pipelineInfo = createInfo.*_pipelineInfo;

	return {
		.paddedDims = createInfo.paddedDims,

		.paddedEcDims = pipelineInfo.paddedEcDims,
		.paddedecDims = pipelineInfo.paddedecDims,

		.dims = createInfo.dims,
		.Cr = createInfo.Cr,

		.dim = createInfo.dim,

		.mu = createInfo.mu,
		.eps = createInfo.eps,

		.Ec = pipelineInfo.Ec,
		.ec = pipelineInfo.ec,

		.sliceIndex = createInfo.sliceIndex,
		.sliceDelta = createInfo.sliceDelta,

		.shaderPath = createInfo.shaderPath,
		.entrypoint = createInfo.entrypoint,
		.workGroupSize = createInfo.workGroupSize,

		.compute = createInfo.compute,
	};
}

template <typename T>
class AbcDoublePipelines
{
public:
	AbcDoublePipelines(const AbcDoublePipelinesCreateInfo<T>& createInfo):
		slice1(map<T>(createInfo, &AbcDoublePipelinesCreateInfo<T>::slice1)),
		slice2(map<T>(createInfo, &AbcDoublePipelinesCreateInfo<T>::slice2))
	{}

	void dispatch(vk::CommandBuffer commandBuffer)
	{
		slice1.dispatch(commandBuffer);
		slice2.dispatch(commandBuffer);
	}

private:
	AbcSlicedPipeline<T> slice1;
	AbcSlicedPipeline<T> slice2;
};

struct AbcDoublePipelinesInfo
{
	vulkan::Buffer& ec1;
	vulkan::Buffer& ec2;
	std::size_t sliceIndex;
	std::ptrdiff_t sliceDelta;
};

template <typename T>
struct Abc01PipelinesCreateInfo
{

	svec3 paddedDims;
	svec3 paddedEc1Dims;
	svec3 paddedEc2Dims;
	svec2 paddedec1Dims;
	svec2 paddedec2Dims;
	svec3 dims;
	T Cr;

	Dim dim;

	vulkan::Buffer& Ec1;
	vulkan::Buffer& Ec2;
	vulkan::Buffer& mu;
	vulkan::Buffer& eps;

	AbcDoublePipelinesInfo bottom;
	AbcDoublePipelinesInfo top;

	std::filesystem::path shaderPath;
	std::string_view      entrypoint = "main";
	svec3                 workGroupSize;

	vulkan::Compute& compute;
};

template <typename T>
AbcDoublePipelinesCreateInfo<T> map(const Abc01PipelinesCreateInfo<T> createInfo, AbcDoublePipelinesInfo Abc01PipelinesCreateInfo<T>::* _pipelineInfo)
{
	auto& pipelineInfo = createInfo.*_pipelineInfo;

	return {

			.paddedDims    = createInfo.paddedDims,
			.dims          = createInfo.dims,
			.Cr            = createInfo.Cr,
			.dim           = createInfo.dim,

			.mu  = createInfo.mu,
			.eps = createInfo.eps,

			.slice1 = {
				.paddedEcDims = createInfo.paddedEc1Dims,
				.paddedecDims = createInfo.paddedec1Dims,
				.Ec           = createInfo.Ec1,
				.ec           = pipelineInfo.ec1,
			},
			.slice2 = {
				.paddedEcDims = createInfo.paddedEc2Dims,
				.paddedecDims = createInfo.paddedec2Dims,
				.Ec           = createInfo.Ec2,
				.ec           = pipelineInfo.ec2,
			},

			.sliceIndex = pipelineInfo.sliceIndex,
			.sliceDelta = pipelineInfo.sliceDelta,

			.shaderPath    = createInfo.shaderPath,
			.entrypoint    = createInfo.entrypoint,
			.workGroupSize = createInfo.workGroupSize,
			.compute       = createInfo.compute,
	};
}

template <typename T>
class Abc01Pipelines
{
public:
	Abc01Pipelines(const Abc01PipelinesCreateInfo<T>& createInfo):
		bottom(map<T>(createInfo, &Abc01PipelinesCreateInfo<T>::bottom)),
		top(map<T>(createInfo, &Abc01PipelinesCreateInfo<T>::top))
	{}

	void dispatch(vk::CommandBuffer commandBuffer)
	{
		bottom.dispatch(commandBuffer);
		top.dispatch(commandBuffer);
	}

private:
	AbcDoublePipelines<T> bottom;
	AbcDoublePipelines<T> top;

};

struct Abc01PipelinesInfo
{
	svec3 paddedEc1Dims;
	svec3 paddedEc2Dims;
	svec2 paddedec1Dims;
	svec2 paddedec2Dims;

	vulkan::Buffer& Ec1;
	vulkan::Buffer& Ec2;
	vulkan::Buffer& mu;
	vulkan::Buffer& eps;
	vulkan::Buffer& ec10;
	vulkan::Buffer& ec11;
	vulkan::Buffer& ec20;
	vulkan::Buffer& ec21;

	std::string_view entrypoint = "main";
};

template <typename T>
struct AbcPipelinesCreateInfo
{
	T Cr;
	svec3 paddedDims;
	svec3 dims;

	std::filesystem::path shaderPath;
	svec3                 workGroupSize;

	vulkan::Compute& compute;

	Abc01PipelinesInfo x;
	Abc01PipelinesInfo y;
	Abc01PipelinesInfo z;

};

std::size_t getSliceIndex(svec3 dims, Dim dim)
{
	switch(dim)
	{
		case Dim::X:
			return dims.x;
		case Dim::Y:
			return dims.y;
		case Dim::Z:
			return dims.z;
	};
}

template <typename T>
Abc01PipelinesCreateInfo<T> map(const AbcPipelinesCreateInfo<T> createInfo, Abc01PipelinesInfo AbcPipelinesCreateInfo<T>::* _pipelineInfo, Dim dim)
{
	auto& pipelineInfo = createInfo.*_pipelineInfo;

	return {
		.paddedDims    = createInfo.paddedDims,
		.paddedEc1Dims = pipelineInfo.paddedEc1Dims,
		.paddedEc2Dims = pipelineInfo.paddedEc2Dims,
		.paddedec1Dims = pipelineInfo.paddedec1Dims,
		.paddedec2Dims = pipelineInfo.paddedec2Dims,
		.dims          = createInfo.dims,
		.Cr            = createInfo.Cr,
		.dim           = dim,
		.Ec1           = pipelineInfo.Ec1,
		.Ec2           = pipelineInfo.Ec2,
		.mu            = pipelineInfo.mu,
		.eps           = pipelineInfo.eps,
		.bottom = {
			.ec1        = pipelineInfo.ec10,
			.ec2        = pipelineInfo.ec20,
			.sliceIndex = 0,
			.sliceDelta = 1,
		},
		.top = {
			.ec1        = pipelineInfo.ec11,
			.ec2        = pipelineInfo.ec21,
			.sliceIndex = getSliceIndex(createInfo.dims, dim)-1,
			.sliceDelta = -1,
		},
		.shaderPath    = createInfo.shaderPath,
		.entrypoint    = pipelineInfo.entrypoint,
		.workGroupSize = slice(createInfo.workGroupSize, dim),
		.compute       = createInfo.compute,
	};
}

template <typename T>
class AbcPipelines
{
public:
	using create_info_t = AbcPipelinesCreateInfo<T>;

	AbcPipelines(create_info_t createInfo):
		x(map<T>(createInfo, &create_info_t::x, Dim::X)),
		y(map<T>(createInfo, &create_info_t::y, Dim::Y)),
		z(map<T>(createInfo, &create_info_t::z, Dim::Z))
	{ }

	void dispatch(vk::CommandBuffer commandBuffer)
	{
		x.dispatch(commandBuffer);
		y.dispatch(commandBuffer);
		z.dispatch(commandBuffer);
	}

private:
	Abc01Pipelines<T> x;
	Abc01Pipelines<T> y;
	Abc01Pipelines<T> z;
};

}
