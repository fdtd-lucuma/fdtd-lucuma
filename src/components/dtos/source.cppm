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

import lucuma.utils;
import std;
import magic_enum;

export module lucuma.components.dtos:source;

import lucuma.legacy_headers.simdjson;

import :gaussian_source;

namespace lucuma::components::dtos
{

export struct Source
{
	utils::SourceType            type;
	std::variant<GaussianSource> source;
};

}

namespace simdjson
{

using namespace lucuma::components::dtos;
using namespace lucuma::utils;

export template <typename simdjson_value>
auto tag_invoke(deserialize_tag, simdjson_value& val, Source& source)
{
	ondemand::object obj;

	auto error = val.get_object().get(obj);

	if(error)
		return error;

	if((error = obj["type"].get(source.type)))
		return error;

	switch(source.type)
	{
		case SourceType::GAUSSIAN:
			source.source = GaussianSource();
			if((error = obj["source"].get(std::get<GaussianSource>(source.source))))
				return error;
			break;
	}

	return simdjson::SUCCESS;
}

}
