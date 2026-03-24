
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

	std::filesystem::path shaderPath;
	std::string_view      entrypoint = "main";

	vulkan::Compute&   compute;
	vulkan::Allocator& allocator;
	vulkan::Device&    device;
};

template <typename T>
class GaussPipeline
{
private:

	GaussPipelineCreateInfo<T> createInfo;

	std::size_t maxWorkgroupSize;

	struct alignas(32)
	{
		alignas(sizeof(svec4)) svec3 paddedDims;
		T time = {};
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

	std::size_t getWorkgroupSize()
	{
		return std::min(maxWorkgroupSize, sourceSsbos.size());
	}

	std::size_t getWorkgroupCount()
	{
		const auto workgroupSize = getWorkgroupSize();
		const auto size          = sourceSsbos.size();

		if(size == 0)
			return 1;

		return size/workgroupSize + ((size % workgroupSize) == 0 ? 0 : 1);
	}

	// TODO: use cache
	vulkan::ComputePipeline recreatePipeline(vulkan::Buffer& Ec)
	{
		if(sourceSsbos.empty())
			return {};

		return createInfo.compute.createPipeline({
			.shaderPath = createInfo.shaderPath,
			.setLayouts = {
				{
					.bindings = simpleStorageBuffersLayout<2>(),
					.buffers = {
						Ec,
						sourceSsbosBuffer,
					}
				}
			},
			.pushConstants = vulkan::Compute::makePushConstantsLayout<typeof(pushConstants)>(),
			.specializationConstants = SpecializationConstants::make(
				0, (std::uint32_t)getWorkgroupSize(),
				1, (std::uint32_t)1,
				2, (std::uint32_t)1,
				4, (std::uint32_t)sourceSsbos.size()
			)
		});
	}

	vulkan::Buffer allocateSsbosBuffer(std::size_t bytes)
	{
		if(bytes == 0)
			return {};

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

	static std::size_t getMaxWorkgroupSize(vulkan::Device& device)
	{
		const auto limits = device.getPhysicalDevice().getProperties().limits;

		return (std::size_t)std::min(limits.maxComputeWorkGroupSize[0], limits.maxComputeWorkGroupInvocations);
	}

public:
	GaussPipeline(const GaussPipelineCreateInfo<T>& createInfo):
		createInfo(createInfo),
		maxWorkgroupSize(getMaxWorkgroupSize(createInfo.device)),
		pushConstants({
			.paddedDims = createInfo.paddedDims,
		}),
		sourceSsbosBuffer(allocateSsbosBuffer(0))
	{

	}

	void fillData(entt::registry& registry, vulkan::Buffer& Ec)
	{
		sourceSsbos.clear();

		auto group = registry.group<const components::GaussianSource<T>>(entt::get<const components::Transform>);

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
			pipeline          = recreatePipeline(Ec);
		}

		sourceSsbosBuffer.memcpy(sourceSsbos.data(), byteSize);

	}

	void dispatch(vk::CommandBuffer commandBuffer, entt::registry& registry, T time, vulkan::Buffer& Ec, T x0 = 0)
	{
		pushConstants.time = time;
		pushConstants.x0   = x0;

		fillData(registry, Ec);

		if(sourceSsbos.empty())
			return;

		pipeline.bind(commandBuffer);
		pipeline.pushConstants(commandBuffer, pushConstants);
		commandBuffer.dispatch(getWorkgroupCount(),1,1);
	}

};

}
