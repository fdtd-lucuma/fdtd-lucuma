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

export struct ComputePipelineCreateInfo
{
	struct setLayout {
		std::span<const vk::DescriptorSetLayoutBinding>       bindings;
		std::span<const std::reference_wrapper<const Buffer>> buffers;
	};

	std::filesystem::path      shaderPath;
	std::string                entrypoint = "main";
	std::span<const setLayout> setLayouts;
};

export class ComputePipeline
{
public:
	std::span<vk::raii::DescriptorSetLayout> getDescriptorSetLayouts();
	std::vector<vk::DescriptorSetLayout>     getDescriptorSetLayoutsUnraii();

	std::span<vk::raii::DescriptorSet> getDescriptorSets();
	std::vector<vk::DescriptorSet>     getDescriptorSetsUnraii();

	vk::raii::DescriptorPool& getDescriptorPool();
	vk::raii::CommandBuffer&  getCommandBuffer();
	vk::raii::PipelineLayout& getLayout();
	vk::raii::Pipeline&       getPipeline();

	ComputePipeline() = default;

	ComputePipeline(ComputePipeline const&) = delete;
	ComputePipeline(ComputePipeline&& other);

	ComputePipeline& operator=(ComputePipeline const&) = delete;
	ComputePipeline& operator=(ComputePipeline&&)      = default;

	void bind(vk::CommandBuffer commandBuffer);

private:
	std::vector<vk::raii::DescriptorSetLayout> descriptorSetLayouts;

	vk::raii::DescriptorPool descriptorPool = nullptr;
	vk::raii::CommandBuffer  commandBuffer  = nullptr;

	std::vector<vk::raii::DescriptorSet> descriptorSets;

	vk::raii::PipelineLayout layout   = nullptr;
	vk::raii::Pipeline       pipeline = nullptr;

	ComputePipeline(Compute& builder, const ComputePipelineCreateInfo& info);

	friend class Compute;
};


class Compute
{
public:
	Compute(Injector& injector);

	vk::raii::Queue&          getQueue();
	vk::raii::CommandPool&    getCommandPool();

	ComputePipeline createPipeline(const ComputePipelineCreateInfo& info);

	void submit(const vk::CommandBuffer& commandBuffer);

private:
	Device&       device;
	ShaderLoader& shaderLoader;

	std::vector<vk::raii::Queue> queues;
	vk::raii::CommandPool        commandPool = nullptr;

	void init();

	void createQueues();
	void createCommandPool();

	friend class ComputePipeline;
};

}
