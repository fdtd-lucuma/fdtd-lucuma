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

export module lucuma.services.backends.vulkan_components:update_e_pipeline;

import lucuma.utils;
import lucuma.services.vulkan;
import vulkan_hpp;
import std;

namespace lucuma::services::backends::vulkan_components
{

using namespace lucuma::utils;

struct UpdateEPipelineCreateInfo
{
	svec3 paddedEDims;
	svec3 paddedHc1Dims;
	svec3 paddedHc2Dims;
	svec3 dims;
	svec3 start;

	svec3Delta Hc1Delta;
	svec3Delta Hc2Delta;

	vulkan::Buffer& Ec;
	vulkan::Buffer& Ce;
	vulkan::Buffer& Ch;
	vulkan::Buffer& Hc1;
	vulkan::Buffer& Hc2;

	std::filesystem::path shaderPath;
	std::string_view      entrypoint = "main";
	svec3                 workGroupSize;

	vulkan::Compute& compute;
};

class UpdateEPipeline
{
private:
	struct alignas(32)
	{
		alignas(sizeof(svec4)) svec3 paddedEDims;
		alignas(sizeof(svec4)) svec3 paddedHc1Dims;
		alignas(sizeof(svec4)) svec3 paddedHc2Dims;
		alignas(sizeof(svec4)) svec3 dims;
		alignas(sizeof(svec4)) svec3 start;
	} pushConstants;

	svec3 groupCount;

	vulkan::ComputePipeline pipeline;

public:
	UpdateEPipeline(const UpdateEPipelineCreateInfo& createInfo);

	void dispatch(vk::CommandBuffer commandBuffer);

};

struct UpdateEPipelineInfo
{
	svec3 paddedEDims;
	svec3 paddedHc1Dims;
	svec3 paddedHc2Dims;
	svec3 start;

	svec3Delta Hc1Delta;
	svec3Delta Hc2Delta;

	vulkan::Buffer& Ec;
	vulkan::Buffer& Ce;
	vulkan::Buffer& Ch;
	vulkan::Buffer& Hc1;
	vulkan::Buffer& Hc2;

	std::string_view entrypoint = "main";
};

struct UpdateEPipelinesCreateInfo
{
	std::filesystem::path shaderPath;
	svec3                 workGroupSize;
	svec3                 dims;

	vulkan::Compute& compute;

	UpdateEPipelineInfo x;
	UpdateEPipelineInfo y;
	UpdateEPipelineInfo z;

};

class UpdateEPipelines
{
public:
	using create_info_t = UpdateEPipelinesCreateInfo;

	UpdateEPipelines(create_info_t createInfo);

	void dispatch(vk::CommandBuffer commandBuffer);

private:
	UpdateEPipeline x;
	UpdateEPipeline y;
	UpdateEPipeline z;
};

}
