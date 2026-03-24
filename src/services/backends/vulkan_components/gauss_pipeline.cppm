
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

export module lucuma.services.backends.vulkan_components:gauss_pipeline;

import glm;

import lucuma.utils;
import lucuma.utils.vulkan;
import lucuma.services.vulkan;
import lucuma.legacy_headers.entt;
import lucuma.components;
import vulkan_hpp;
import vk_mem_alloc;

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
	vulkan::Allocator& allocator;
};

template <typename T>
class GaussPipeline
{
private:

	GaussPipelineCreateInfo<T> createInfo;

	struct alignas(32)
	{
		alignas(sizeof(svec4)) svec3 paddedDims;
		alignas(sizeof(svec4)) svec3 pos = {};
		T time = {};
		T sigma = {};
		T x0 = {};
	} pushConstants;

	struct SourceSsbo
	{
		alignas(sizeof(svec4)) svec3 position = {};
		T sigma;
	};

	std::vector<SourceSsbo> sourceSsbos;
	vulkan::Buffer          sourceSsbosBuffer;

	vulkan::ComputePipeline pipeline;

	// TODO: use cache
	vulkan::ComputePipeline recreatePipeline()
	{
		return createInfo.compute.createPipeline({
			.shaderPath = createInfo.shaderPath,
			.setLayouts = {
				{
					.bindings = simpleStorageBuffersLayout<2>(),
					.buffers = {
						createInfo.Ec,
						sourceSsbosBuffer,
					}
				}
			},
			.pushConstants = vulkan::Compute::makePushConstantsLayout<typeof(pushConstants)>(),
		});
	}

	vulkan::Buffer allocateSsbosBuffer(std::size_t bytes)
	{
		vma::AllocationCreateFlags vmaFlags =
			vma::AllocationCreateFlagBits::eMapped |
			vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;

		return createInfo.allocator.allocate(
			bytes,
			vk::BufferUsageFlagBits::eStorageBuffer,
			vmaFlags,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);
	}

public:
	GaussPipeline(const GaussPipelineCreateInfo<T>& createInfo):
		createInfo(createInfo),
		pushConstants({
			.paddedDims = createInfo.paddedDims,
		}),
		sourceSsbosBuffer(allocateSsbosBuffer(1*sizeof(SourceSsbo))),
		pipeline(recreatePipeline())
	{ }

	void fillData(const entt::registry& registry)
	{
		sourceSsbos.clear();

		auto group = registry.group<components::GaussianSource<T>>(entt::get<components::Transform>);

		for(auto&& [_, source, transform]: group.each())
		{
			sourceSsbos.emplace_back(SourceSsbo{
				.position = transform.position,
				.sigma    = source.sigma,
			});
		}

		const std::size_t byteSize = sourceSsbos.size()*sizeof(SourceSsbo);

		if(byteSize > sourceSsbosBuffer.getInfo().size)
		{
			sourceSsbosBuffer = allocateSsbosBuffer(byteSize);
			pipeline          = recreatePipeline();
		}

		sourceSsbosBuffer.memcpy(sourceSsbos.data(), byteSize);

	}

	void dispatch(vk::CommandBuffer, const entt::registry& registry, T time, T x0 = 0)
	{
		pushConstants.time = time;
		pushConstants.x0   = x0;

		fillData(registry);
	}

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
