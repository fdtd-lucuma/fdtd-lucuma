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

export module lucuma.services.backends.vulkan_components:update_h_pipeline;

import lucuma.utils;
import lucuma.services.vulkan;
import vulkan_hpp;

namespace lucuma::services::backends::vulkan_components
{

using namespace lucuma::utils;

struct UpdateHPipelineCreateInfo
{
	svec3 paddedHDims;
	svec3 paddedEc1Dims;
	svec3 paddedEc2Dims;
	svec3 HDims;

	svec3 Ec1Delta;
	svec3 Ec2Delta;

	vulkan::Buffer& Hc;
	vulkan::Buffer& Ch;
	vulkan::Buffer& Ce;
	vulkan::Buffer& Ec1;
	vulkan::Buffer& Ec2;

	std::filesystem::path shaderPath;
	std::string_view      entrypoint = "main";
	svec3                 workGroupSize;

	vulkan::Compute& compute;
};

class UpdateHPipeline
{
private:
	struct alignas(32)
	{
		alignas(sizeof(svec4)) svec3 paddedHDims;
		alignas(sizeof(svec4)) svec3 paddedEc1Dims;
		alignas(sizeof(svec4)) svec3 paddedEc2Dims;
		alignas(sizeof(svec4)) svec3 HDims;
	} pushConstants;

	svec3 groupCount;

	vulkan::ComputePipeline pipeline;

public:
	UpdateHPipeline(const UpdateHPipelineCreateInfo& createInfo);

	void dispatch(vk::CommandBuffer commandBuffer);

};

struct UpdateHPipelineInfo
{
	svec3 paddedHDims;
	svec3 paddedEc1Dims;
	svec3 paddedEc2Dims;
	svec3 HDims;

	svec3 Ec1Delta;
	svec3 Ec2Delta;

	vulkan::Buffer& Hc;
	vulkan::Buffer& Ch;
	vulkan::Buffer& Ce;
	vulkan::Buffer& Ec1;
	vulkan::Buffer& Ec2;

	std::string_view entrypoint = "main";
};

struct UpdateHPipelinesCreateInfo
{
	std::filesystem::path shaderPath;
	svec3                 workGroupSize;

	vulkan::Compute& compute;

	UpdateHPipelineInfo x;
	UpdateHPipelineInfo y;
	UpdateHPipelineInfo z;

};

class UpdateHPipelines
{
public:
	using create_info_t = UpdateHPipelinesCreateInfo;

	UpdateHPipelines(create_info_t createInfo);

	void dispatch(vk::CommandBuffer commandBuffer);

private:
	UpdateHPipeline x;
	UpdateHPipeline y;
	UpdateHPipeline z;
};

}
