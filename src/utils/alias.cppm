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

export module lucuma.utils:alias;

import std;
import glm;

import lucuma.legacy_headers.simdjson;

namespace lucuma::utils
{

/// Size vector 1 for fdtd matrices
export using svec1 = glm::vec<1, std::uint64_t>;

/// Size vector 2 for fdtd matrices
export using svec2 = glm::vec<2, std::uint64_t>;

/// Size vector 3 for fdtd matrices
export using svec3 = glm::vec<3, std::uint64_t>;

/// Size vector 4 for fdtd matrices
export using svec4 = glm::vec<4, std::uint64_t>;

/// Size vector 1 for fdtd matrices deltas
export using svec1Delta = glm::vec<1, std::ptrdiff_t>;

/// Size vector 2 for fdtd matrices deltas
export using svec2Delta = glm::vec<2, std::ptrdiff_t>;

/// Size vector 3 for fdtd matrices deltas
export using svec3Delta = glm::vec<3, std::ptrdiff_t>;

/// Size vector 4 for fdtd matrices deltas
export using svec4Delta = glm::vec<4, std::ptrdiff_t>;

}

namespace simdjson
{

using namespace lucuma::utils;

// TODO: Delete this once C++26 reflection becomes mainstream

export template <typename simdjson_value>
auto tag_invoke(deserialize_tag, simdjson_value& val, svec1& vec)
{
	ondemand::object obj;

	auto error = val.get_object().get(obj);

	if(error)
		return error;

	if((error = obj["x"].get(vec.x)))
		return error;

	return simdjson::SUCCESS;
}

export template <typename simdjson_value>
auto tag_invoke(deserialize_tag, simdjson_value& val, svec1Delta& vec)
{
	ondemand::object obj;

	auto error = val.get_object().get(obj);

	if(error)
		return error;

	if((error = obj["x"].get(vec.x)))
		return error;

	return simdjson::SUCCESS;
}

export template <typename simdjson_value>
auto tag_invoke(deserialize_tag, simdjson_value& val, svec2& vec)
{
	ondemand::object obj;

	auto error = val.get_object().get(obj);

	if(error)
		return error;

	if((error = obj["x"].get(vec.x)))
		return error;

	if((error = obj["y"].get(vec.y)))
		return error;

	return simdjson::SUCCESS;
}

export template <typename simdjson_value>
auto tag_invoke(deserialize_tag, simdjson_value& val, svec2Delta& vec)
{
	ondemand::object obj;

	auto error = val.get_object().get(obj);

	if(error)
		return error;

	if((error = obj["x"].get(vec.x)))
		return error;

	if((error = obj["y"].get(vec.y)))
		return error;

	return simdjson::SUCCESS;
}

export template <typename simdjson_value>
auto tag_invoke(deserialize_tag, simdjson_value& val, svec3& vec)
{
	ondemand::object obj;

	auto error = val.get_object().get(obj);

	if(error)
		return error;

	if((error = obj["x"].get(vec.x)))
		return error;

	if((error = obj["y"].get(vec.y)))
		return error;

	if((error = obj["z"].get(vec.z)))
		return error;

	return simdjson::SUCCESS;
}

export template <typename simdjson_value>
auto tag_invoke(deserialize_tag, simdjson_value& val, svec3Delta& vec)
{
	ondemand::object obj;

	auto error = val.get_object().get(obj);

	if(error)
		return error;

	if((error = obj["x"].get(vec.x)))
		return error;

	if((error = obj["y"].get(vec.y)))
		return error;

	if((error = obj["z"].get(vec.z)))
		return error;

	return simdjson::SUCCESS;
}

}
