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

export module fdtd.services.vulkan:shader_loader;

import vulkan_hpp;
import std;

import fdtd.utils;
import fdtd.services;

import :device;
import :utils;

export class VulkanShaderLoader
{
public:
	VulkanShaderLoader(Injector& injector);

private:
	VulkanDevice& vulkanDevice;
	FileReader&   fileReader;

	ReturnType<vk::raii::ShaderModule> createShaderModule(const std::filesystem::path& path);

};
