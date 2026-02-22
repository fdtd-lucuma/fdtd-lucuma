// Una GUI para fdtd
// Copyright © 2025-2026 Otreblan
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

export module lucuma.utils.imgui:combo;

import imgui;
import magic_enum;
import std;

namespace lucuma::utils::imgui
{

template <std::size_t N>
constexpr std::array<const char*, N> stringView2Ptr(const std::array<std::string_view, N> strs)
{
	std::array<const char*, N> result;

	for(std::size_t i = 0; i < strs.size(); i++)
	{
		result[i] = strs[i].data();
	}

	return result;
}

export template <typename T>
requires std::is_enum_v<T>
bool Combo(std::string_view label, T* v)
{
	int item_current = 0;

	constexpr auto strViewValues = magic_enum::enum_names<T>();
	constexpr auto ptrValues = stringView2Ptr(strViewValues);

	if(v)
	{
		const auto name = magic_enum::enum_name(*v);

		const auto it = std::ranges::find(strViewValues, name);

		if(it != strViewValues.end())
		{
			item_current = std::distance(strViewValues.begin(), it);
		}
	}

	bool changed = ::ImGui::Combo(label.data(), &item_current, ptrValues.data(), ptrValues.size());

	if(v)
	{
		auto casted = magic_enum::enum_cast<T>(ptrValues[item_current]);
		if(casted.has_value())
		{
			*v = casted.value();
		}
	}

	return changed;
}

}

