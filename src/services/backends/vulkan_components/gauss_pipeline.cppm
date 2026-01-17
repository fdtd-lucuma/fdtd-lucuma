
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

export module lucuma.services.backends.vulkan_components:gauss_pipeline;

import glm;

import lucuma.utils;
import lucuma.utils.vulkan;
import lucuma.services.vulkan;
import vulkan;

import std;

import :utils;

namespace lucuma::services::backends::vulkan_components
{

using namespace lucuma::utils;
using namespace lucuma::utils::vulkan;

template <typename T>
struct GaussPipelineCreateInfo
{
	svec3 paddedDims;

	vulkan::Buffer& Ec;

	std::filesystem::path shaderPath;
	std::string_view      entrypoint = "main";

	vulkan::Compute& compute;
};

template <typename T>
class GaussPipeline
{
private:
	struct alignas(32)
	{
		alignas(sizeof(svec4)) svec3 paddedDims;
		alignas(sizeof(svec4)) svec3 pos = {};
		T time = {};
		T sigma = {};
		T x0 = {};
	} pushConstants;

	vulkan::ComputePipeline pipeline;

public:
	GaussPipeline(const GaussPipelineCreateInfo<T>& createInfo):
		pushConstants({
			.paddedDims = createInfo.paddedDims,
		}),
		pipeline(createInfo.compute.createPipeline({
			.shaderPath = createInfo.shaderPath,
			.setLayouts = {
				{
					.bindings = simpleStorageBuffersLayout<1>(),
					.buffers = {
						createInfo.Ec,
					}
				}
			},
			.pushConstants = vulkan::Compute::makePushConstantsLayout<typeof(pushConstants)>(),
		}))
	{ }

	void dispatch(vk::CommandBuffer commandBuffer, svec3 pos, T time, T sigma, T x0 = 0)
	{
		pushConstants.pos   = pos;
		pushConstants.time  = time;
		pushConstants.sigma = sigma;
		pushConstants.x0    = x0;

		pipeline.bind(commandBuffer);
		pipeline.pushConstants(commandBuffer, pushConstants);
		commandBuffer.dispatch(1,1,1);
	}

};

}
