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

export module lucuma.services.vulkan:compute;

import vulkan_hpp;
import std;

import lucuma.utils;

namespace lucuma::services::vulkan
{

using namespace lucuma::utils;
using namespace lucuma::services;

class Device;
class ShaderLoader;
class Buffer;

export class Compute;

export class SimpleCommandBuffer
{
public:
	SimpleCommandBuffer() = default;

	SimpleCommandBuffer(SimpleCommandBuffer const&) = delete;
	SimpleCommandBuffer(SimpleCommandBuffer&& other);

	SimpleCommandBuffer& operator=(SimpleCommandBuffer const&) = delete;
	SimpleCommandBuffer& operator=(SimpleCommandBuffer&&)      = default;

	vk::raii::CommandBuffer& getCommandBuffer();

	operator vk::CommandBuffer();
	vk::raii::CommandBuffer* operator ->();
private:
	SimpleCommandBuffer(Compute& compute);

	vk::raii::CommandBuffer commandBuffer = nullptr;

	friend class Compute;
};

export struct ComputePipelineCreateInfo
{
	struct setLayout {
		std::span<const vk::DescriptorSetLayoutBinding>       bindings;
		std::span<const std::reference_wrapper<const Buffer>> buffers;
	};

	std::filesystem::path      shaderPath;
	std::string                entrypoint = "main";
	std::span<const setLayout> setLayouts;
	svec3                      workGroupSize = {1,1,1};

	std::span<const vk::PushConstantRange> pushConstants;
};

export class ComputePipeline
{
public:
	std::span<vk::raii::DescriptorSetLayout> getDescriptorSetLayouts();
	std::vector<vk::DescriptorSetLayout>     getDescriptorSetLayoutsUnraii();

	std::span<vk::raii::DescriptorSet> getDescriptorSets();
	std::vector<vk::DescriptorSet>     getDescriptorSetsUnraii();

	vk::raii::DescriptorPool& getDescriptorPool();
	vk::raii::PipelineLayout& getLayout();
	vk::raii::Pipeline&       getPipeline();

	ComputePipeline() = default;

	ComputePipeline(ComputePipeline const&) = delete;
	ComputePipeline(ComputePipeline&& other);

	ComputePipeline& operator=(ComputePipeline const&) = delete;
	ComputePipeline& operator=(ComputePipeline&&)      = default;

	void bind(vk::CommandBuffer commandBuffer);

	template <typename T>
	requires std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && (alignof(T) % 4 == 0) // std430
	void pushConstants(vk::CommandBuffer commandBuffer, const T& data, std::uint32_t offset = 0)
	{
		commandBuffer.pushConstants(getLayout(), vk::ShaderStageFlagBits::eCompute, offset, data);
	}

private:

	std::vector<vk::raii::DescriptorSetLayout> descriptorSetLayouts;

	vk::raii::DescriptorPool descriptorPool = nullptr;

	std::vector<vk::raii::DescriptorSet> descriptorSets;

	vk::raii::PipelineLayout layout   = nullptr;
	vk::raii::Pipeline       pipeline = nullptr;

	ComputePipeline(Compute& builder, const ComputePipelineCreateInfo& info);

	friend class Compute;
};

struct CommandRecorderCreateInfo
{
	vk::CommandBuffer commandBuffer;
	Compute&          compute;
};

/// A wrapper around a command buffer that submits to the Compute service
/// on deletion.
export class CommandRecorder
{
public:
	vk::CommandBuffer& getCommandBuffer();
	operator vk::CommandBuffer&();
	vk::CommandBuffer* operator ->();

	~CommandRecorder();

	CommandRecorder() = default;

	CommandRecorder(CommandRecorder const&) = delete;
	CommandRecorder(CommandRecorder&& other);

	CommandRecorder& operator=(CommandRecorder const&) = delete;
	CommandRecorder& operator=(CommandRecorder&&)      = default;

private:
	CommandRecorder(const CommandRecorderCreateInfo& createInfo);

	void init();

	vk::CommandBuffer commandBuffer = {};
	Compute*          compute = nullptr;

	friend class Compute;
};


class Compute
{
public:
	Compute(Injector& injector);

	vk::raii::Queue&          getQueue();
	vk::raii::CommandPool&    getCommandPool();

	ComputePipeline createPipeline(const ComputePipelineCreateInfo& info);
	SimpleCommandBuffer createSimpleCommandBuffer();
	CommandRecorder createCommandRecorder(vk::CommandBuffer commandBuffer);

	void submit(const vk::CommandBuffer& commandBuffer);
	svec3 getWorkgroupSize(svec3 size) const;

	template<typename T, typename... Members>
	static constexpr auto makePushConstantsLayout(Members... members)
	{
		return std::array {
			vk::PushConstantRange {
				vk::ShaderStageFlagBits::eCompute,
				(std::uint32_t)
				reinterpret_cast<std::size_t>(
					&(reinterpret_cast<T const volatile*>(0)->*members)
				)
				,
				sizeof(std::remove_reference_t<
					decltype(((T*)0)->*members)
				>)
			}...
		};
	}


private:
	Device&       device;
	ShaderLoader& shaderLoader;

	std::vector<vk::raii::Queue> queues;
	vk::raii::CommandPool        commandPool = nullptr;

	void init();

	void createQueues();
	void createCommandPool();

	// TODO: Better wrapping
	friend class ComputePipeline;
	friend class SimpleCommandBuffer;
};

}
