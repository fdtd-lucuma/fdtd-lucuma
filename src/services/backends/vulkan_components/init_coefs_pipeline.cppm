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

		constexpr auto getPushConstantsLayout()
		{
			using this_t = typeof(pushConstants);

			return vulkan::Compute::makePushConstantsLayout<this_t>(
				&this_t::paddedDims,
				&this_t::dims,
				&this_t::CrImp
			);
		}

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
			.pushConstants = pushConstants.getPushConstantsLayout(),
		}))
	{ }

	void dispatch(vk::CommandBuffer commandBuffer)
	{
		pipeline.bind(commandBuffer);
		pipeline.pushConstants(commandBuffer, pushConstants);
		commandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
	}

};

}
