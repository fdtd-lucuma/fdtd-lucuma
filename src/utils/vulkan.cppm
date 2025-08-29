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

export module fdtd.utils:vulkan;

import magic_enum;
import vulkan_hpp;
import std;

/// Put Vulkan bitset enum template specializations here
template <>
struct magic_enum::customize::enum_range<vk::QueueFlagBits> {
	static constexpr bool is_flags = true;
};

export template<typename T>
requires std::is_enum_v<T> && (magic_enum::customize::enum_range<T>::is_flags == true)
std::ostream& operator<<(std::ostream& output, vk::Flags<T> flags)
{
	using magic_enum::iostream_operators::operator<<;

	using MaskType = typename vk::Flags<T>::MaskType;

	return output << (T)(MaskType)flags;
}

export void listVulkanExtensions();

export std::ostream& operator<<(std::ostream& output, vk::Instance instance);
