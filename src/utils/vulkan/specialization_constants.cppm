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

export module lucuma.utils.vulkan:specialization_constants;

import vulkan;
import std;

import lucuma.utils;

namespace lucuma::utils::vulkan
{

export class SpecializationConstants
{
public:
	SpecializationConstants() = default;

	vk::SpecializationInfo getInfo() const;

	template<typename... Args>
	static SpecializationConstants make(Args&&... args)
	{
		static_assert(sizeof...(args) % 2 == 0,
			"Arguments must be pairs: (constantID, value).");

		SpecializationConstants result;
		result.appendAll(std::forward<Args>(args)...);
		return result;
	}

private:
	std::vector<vk::SpecializationMapEntry> entries;
	std::vector<std::byte> data;

	template<typename T, typename... Rest>
	void appendAll(std::uint32_t id, const T& value, Rest&&... rest)
	{
		append(id, value);
		if constexpr (sizeof...(rest) > 0)
			appendAll(std::forward<Rest>(rest)...);
	}

	template<typename T>
	requires (std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T>)
	void append(std::uint32_t id, const T& value)
	{
		std::uint32_t offset = data.size();
		std::size_t size     = sizeof(T);

		vk::SpecializationMapEntry entry {
			.constantID = id,
			.offset     = offset,
			.size       = size,
		};

		entries.push_back(entry);

		const std::byte* ptr = reinterpret_cast<const std::byte*>(&value);
		data.insert(data.end(), ptr, ptr + size);
	}

};

}
