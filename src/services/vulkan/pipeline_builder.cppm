// Una GUI para fdtd
// Copyright © 2025 Otreblan
//
// fdtd-vulkan is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// fdtd-vulkan is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fdtd-vulkan.  If not, see <http://www.gnu.org/licenses/>.

module;

export module fdtd.services.vulkan:pipeline_builder;

import vulkan_hpp;
import std;

import fdtd.utils;

namespace fdtd::services::vulkan
{

using namespace fdtd::utils;

export class PipelineBuilder;

export struct ComputePipelineCreateInfo
{
	struct setLayout {
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
	};

	std::filesystem::path  shaderPath;
	std::string            entrypoint = "main";
	std::vector<setLayout> setLayouts;
};

export class ComputePipeline
{
public:
	std::span<vk::raii::DescriptorSetLayout> getDescriptorSetLayouts();
	std::vector<vk::DescriptorSetLayout>     getDescriptorSetLayoutsUnraii();

	vk::raii::PipelineLayout& getLayout();
	vk::raii::Pipeline&       getPipeline();

private:
	std::vector<vk::raii::DescriptorSetLayout> descriptorSetLayouts;

	vk::raii::PipelineLayout layout   = nullptr;
	vk::raii::Pipeline       pipeline = nullptr;

	ComputePipeline(PipelineBuilder& builder, const ComputePipelineCreateInfo& info);

	friend class PipelineBuilder;
};

class Device;
class ShaderLoader;

class PipelineBuilder
{
public:
	PipelineBuilder(Injector& injector);

	ComputePipeline createComputePipeline(const ComputePipelineCreateInfo& info);

private:
	Device&       device;
	ShaderLoader& shaderLoader;

	friend class ComputePipeline;
};

}

// Explicit template instantiations for faster compilation
namespace fdtd::utils
{
using namespace fdtd::services::vulkan;

extern template PipelineBuilder& Injector::inject<PipelineBuilder>();

}
