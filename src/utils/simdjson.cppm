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

export module lucuma.utils:simdjson;

import lucuma.legacy_headers.simdjson;
import std;
import magic_enum;

namespace simdjson
{

export template <typename T>
requires std::is_enum_v<T>
error_code tag_invoke(deserialize_tag, auto &val, T &out) noexcept
{
	std::string_view str;

	if(auto error = val.get_string(str))
		return error;

	auto opt = magic_enum::enum_cast<T>(str);

	if(!opt.has_value())
		return simdjson::INCORRECT_TYPE;

	out = opt.value();

	return simdjson::SUCCESS;
}

export template <typename ...T>
auto tag_invoke(deserialize_tag, auto& val, std::variant<T...>& variant)
{
	if(variant.valueless_by_exception())
		return simdjson::UNINITIALIZED;

	simdjson::error_code error;

	std::visit([&](auto&& v)
	{
		error = val.get(v);

	}, variant);

	if(error)
		return error;

	return simdjson::SUCCESS;
}

}
