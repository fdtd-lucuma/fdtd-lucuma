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

export module lucuma.services.backends.vulkan_components:utils;

import std;

import lucuma.utils;

import vulkan_hpp;

namespace lucuma::services::backends::vulkan_components
{

using namespace lucuma::utils;

template <std::size_t N>
constexpr auto simpleStorageBuffersLayout()
{
	return []<std::size_t... I>(std::index_sequence<I...>)
	{
		return std::array{
			vk::DescriptorSetLayoutBinding{
				I,
				vk::DescriptorType::eStorageBuffer,
				1,
				vk::ShaderStageFlagBits::eCompute
			}...
		};
	}(std::make_index_sequence<N>{});
}

svec3 pad(svec3 size, svec3 workGroupSize);
svec2 pad(svec2 size, svec2 workGroupSize);

template <typename T>
constexpr std::string_view shaderSuffix()
{
	if constexpr(std::is_same_v<T, PrecisionTraits<Precision::f16>::type>)
		return "_half.spv";
	if constexpr(std::is_same_v<T, PrecisionTraits<Precision::f32>::type>)
		return "_float.spv";
	if constexpr(std::is_same_v<T, PrecisionTraits<Precision::f64>::type>)
		return "_double.spv";
}


template <typename T>
std::filesystem::path shaderName(std::string_view name)
{
	return std::format("{}{}", name, shaderSuffix<T>());
}

enum class FieldType
{
	/// Magnetic field
	H,

	/// Electric field
	E,
};

template <FieldType F, typename T>
constexpr T crImp(T Cr, T Imp0)
{
	if constexpr(F == FieldType::H)
		return Cr/Imp0;
	if constexpr(F == FieldType::E)
		return Cr*Imp0;
}


}
