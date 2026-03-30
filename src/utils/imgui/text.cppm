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

export module lucuma.utils.imgui:text;

import imgui;
import magic_enum;
import std;

namespace lucuma::utils::imgui
{

export template <typename T>
requires std::is_enum_v<T>
void Enum(T v)
{
	ImGui::Text("%s", magic_enum::enum_name(v).begin());
}

export template <std::size_t size = 64, typename char_t = char>
struct StackStr
{
	char_t str[size];

	template <typename... Args>
	StackStr(std::format_string<Args...> fmt, Args&&... args)
	{
		const auto result = std::format_to_n(str, size-1, fmt, std::forward<Args>(args)...);
		*result.out = '\0';
	}

	template <typename T>
	StackStr(T&& arg):
		StackStr("{}", std::forward<T>(arg))
	{
	}

	operator const char_t*() const
	{
		return str;
	}

	operator std::basic_string_view<char_t>() const
	{
		return str;
	}

};

export void TextView(std::string_view str);

}
