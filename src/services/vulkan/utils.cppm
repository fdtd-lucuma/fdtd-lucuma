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

export module fdtd.services.vulkan:utils;

import std;
import vulkan_hpp;

import fdtd.services;

namespace fdtd::services::vulkan
{

template<typename T = std::uint32_t>
vk::ArrayProxyNoTemporaries<const T> to_proxy(const FileBuffer& buffer)
{
	const auto span = buffer.getBuffer<T>();

	return {span};
}

template<typename T = std::uint32_t>
vk::ArrayProxyNoTemporaries<const T> to_proxy(FileBuffer&& buffer) = delete;

template<typename RAIIT, typename T = RAIIT::CppType>
std::vector<T> unraii(std::span<RAIIT> raiis) {
	return raiis |
		std::views::transform(&RAIIT::operator T) |
		std::ranges::to<std::vector>();
}

template<typename RAIIT, typename T = RAIIT::CppType>
std::vector<T> unraii(const std::vector<RAIIT>& raiis) {
	return unraii(std::span{raiis});
}

void list_vulkan_extensions();

std::ostream& operator<<(std::ostream& output, vk::Instance instance);
std::ostream& operator<<(std::ostream& output, vk::PhysicalDevice physicalDevice);
std::ostream& operator<<(std::ostream& output, vk::QueueFamilyProperties properties);

}
