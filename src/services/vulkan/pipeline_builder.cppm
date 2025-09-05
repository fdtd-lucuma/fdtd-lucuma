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

import :device;
import :shader_loader;

export class VulkanPipelineBuilder;

export class VulkanComputePipelineData
{
public:

private:
	vk::raii::Pipeline pipeline = nullptr;

	VulkanComputePipelineData(VulkanPipelineBuilder& builder);

	friend class VulkanPipelineBuilder;
};

class VulkanPipelineBuilder
{
public:
	VulkanPipelineBuilder(Injector& injector);

	VulkanComputePipelineData createComputePipeline();

private:
	VulkanDevice&       vulkanDevice;
	VulkanShaderLoader& vulkanShaderLoader;

	friend class VulkanComputePipelineData;
};
