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

export class SpecializationConstants
{
public:
	SpecializationConstants() = default;

	vk::SpecializationInfo getInfo() const;

	template<typename... Args>
	static SpecializationConstants make(Args&&... args)
	{
		static_assert(sizeof...(args) % 2 == 0,
			"Arguments must be pairs: (constantID, value).");

		SpecializationConstants result;
		result.appendAll(std::forward<Args>(args)...);
		return result;
	}

private:
	std::vector<vk::SpecializationMapEntry> entries;
	std::vector<std::byte> data;

	template<typename T, typename... Rest>
	void appendAll(std::uint32_t id, const T& value, Rest&&... rest)
	{
		append(id, value);
		if constexpr (sizeof...(rest) > 0)
			appendAll(std::forward<Rest>(rest)...);
	}

	template<typename T>
	void append(std::uint32_t id, const T& value)
	{
		std::uint32_t offset = data.size();
		std::size_t size   = sizeof(T);

		vk::SpecializationMapEntry entry {
			.constantID = id,
			.offset	 = offset,
			.size	   = size,
		};

		entries.push_back(entry);

		const std::byte* ptr = reinterpret_cast<const std::byte*>(&value);
		data.insert(data.end(), ptr, ptr + size);
	}

};

export class SimpleCommandBuffer
{
public:
	SimpleCommandBuffer() = default;

	SimpleCommandBuffer(SimpleCommandBuffer const&) = delete;
	SimpleCommandBuffer(SimpleCommandBuffer&& other);

	SimpleCommandBuffer& operator=(SimpleCommandBuffer const&) = delete;
	SimpleCommandBuffer& operator=(SimpleCommandBuffer&&)	  = default;

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
		std::span<const vk::DescriptorSetLayoutBinding>	   bindings;
		std::span<const std::reference_wrapper<const Buffer>> buffers;
	};

	std::filesystem::path	  shaderPath;
	std::string				entrypoint = "main";
	std::span<const setLayout> setLayouts;

	std::span<const vk::PushConstantRange> pushConstants = {};
	SpecializationConstants specializationConstants = {};
};

export class ComputePipeline
{
public:
	std::span<vk::raii::DescriptorSetLayout> getDescriptorSetLayouts();
	std::vector<vk::DescriptorSetLayout>	 getDescriptorSetLayoutsUnraii();

	std::span<vk::raii::DescriptorSet> getDescriptorSets();
	std::vector<vk::DescriptorSet>	 getDescriptorSetsUnraii();

	vk::raii::DescriptorPool& getDescriptorPool();
	vk::raii::PipelineLayout& getLayout();
	vk::raii::Pipeline&	   getPipeline();

	ComputePipeline() = default;

	ComputePipeline(ComputePipeline const&) = delete;
	ComputePipeline(ComputePipeline&& other);

	ComputePipeline& operator=(ComputePipeline const&) = delete;
	ComputePipeline& operator=(ComputePipeline&&)	  = default;

	void bind(vk::CommandBuffer commandBuffer);

	template <typename T>
	requires std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && (alignof(T) % 4 == 0) // std430
	void pushConstants(vk::CommandBuffer commandBuffer, const T& data, std::uint32_t offset = 0)
	{
		commandBuffer.pushConstants<T>(getLayout(), vk::ShaderStageFlagBits::eCompute, offset, data);
	}

private:

	std::vector<vk::raii::DescriptorSetLayout> descriptorSetLayouts;

	vk::raii::DescriptorPool descriptorPool = nullptr;

	std::vector<vk::raii::DescriptorSet> descriptorSets;

	vk::raii::PipelineLayout layout   = nullptr;
	vk::raii::Pipeline	   pipeline = nullptr;

	ComputePipeline(Compute& builder, const ComputePipelineCreateInfo& info);

	friend class Compute;
};

struct CommandRecorderCreateInfo
{
	vk::CommandBuffer commandBuffer;
	Compute&		  compute;
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
	CommandRecorder& operator=(CommandRecorder&&)	  = default;

private:
	CommandRecorder(const CommandRecorderCreateInfo& createInfo);

	void init();

	vk::CommandBuffer commandBuffer = {};
	Compute*		  compute = nullptr;

	friend class Compute;
};


class Compute
{
public:
	Compute(Injector& injector);

	vk::raii::Queue&		  getQueue();
	vk::raii::CommandPool&	getCommandPool();

	ComputePipeline createPipeline(const ComputePipelineCreateInfo& info);
	SimpleCommandBuffer createSimpleCommandBuffer();
	CommandRecorder createCommandRecorder(vk::CommandBuffer commandBuffer);

	void submit(const vk::CommandBuffer& commandBuffer);
	svec3 getWorkgroupSize(svec3 size) const;

	template<typename T>
	static constexpr std::array<vk::PushConstantRange, 1> makePushConstantsLayout()
	{
		return std::array {
			vk::PushConstantRange {
				.stageFlags = vk::ShaderStageFlagBits::eCompute,
				.offset = 0,
				.size = sizeof(T),
			}
		};
	}


private:
	Device&	   device;
	ShaderLoader& shaderLoader;

	std::vector<vk::raii::Queue> queues;
	vk::raii::CommandPool		commandPool = nullptr;

	void init();

	void createQueues();
	void createCommandPool();

	// TODO: Better wrapping
	friend class ComputePipeline;
	friend class SimpleCommandBuffer;
};

}
