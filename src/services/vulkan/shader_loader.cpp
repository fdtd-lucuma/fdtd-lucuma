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

template <typename T>
void assertAligned(const void* ptr) {
	assert(reinterpret_cast<std::uintptr_t>(ptr) % alignof(T) == 0 && "Pointer is not properly aligned for T");
}

vk::raii::ShaderModule VulkanShaderLoader::createShaderModule(const std::filesystem::path& path)
{
	auto buffer = fileReader.read(path);

	assertAligned<uint32_t>(buffer.getData());

	vk::ShaderModuleCreateInfo createInfo {
		.codeSize = buffer.getSize() * sizeof(char),
		.pCode    = (const uint32_t*)buffer.getData(),
	};

	return vulkanDevice.getDevice().createShaderModule(createInfo);
}
