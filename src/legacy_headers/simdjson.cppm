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

#include <simdjson.h>

export module lucuma.legacy_headers.simdjson;

#define ONDEMAND SIMDJSON_BUILTIN_IMPLEMENTATION::ondemand

export namespace simdjson
{

using simdjson::padded_string;
using simdjson::padded_string_view;
using simdjson::padded_string_builder;

using simdjson::deserialize_tag;
using simdjson::error_code;
using simdjson::error_message;

using simdjson::tag_invoke;

namespace ONDEMAND
{

	using ONDEMAND::parser;
	using ONDEMAND::document;
	using ONDEMAND::object;
	using ONDEMAND::array;
	using ONDEMAND::json_type;

};

namespace ondemand = ONDEMAND;

};

