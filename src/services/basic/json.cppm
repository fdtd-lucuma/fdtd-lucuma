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

export module lucuma.services.basic:json;


import lucuma.utils;

import std;

import lucuma.legacy_headers.simdjson;

namespace lucuma::services::basic
{

using namespace lucuma::utils;

export class Json
{
public:
	Json(Injector& injector);

	template <typename T>
	inline T parse(std::string_view path)
	{
		T result;

		auto json = simdjson::padded_string::load(path);
		auto doc  = parser.iterate(json);

		if(auto error = doc.get(result))
		{
			std::println(std::cerr, "{}", simdjson::error_message(error));
			throw new std::runtime_error(simdjson::error_message(error));
		}

		return result;
	}

private:
	void init();

	simdjson::ondemand::parser parser;

};

}
