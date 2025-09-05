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

#include <cassert>
#include <cstdint>

module fdtd.services.vulkan;

import fdtd.services;

VulkanShaderLoader::VulkanShaderLoader(Injector& injector):
	vulkanDevice(injector.inject<VulkanDevice>()),
	fileReader(injector.inject<FileReader>())
{}

vk::raii::ShaderModule VulkanShaderLoader::createShaderModule(const std::filesystem::path& path)
{
	auto buffer = fileReader.read(path);

	vk::ShaderModuleCreateInfo createInfo {};
	createInfo.setCode(to_proxy(buffer));

	return vulkanDevice.getDevice().createShaderModule(createInfo);
}
