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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <entt/entt.hpp>
#pragma GCC diagnostic pop

export module lucuma.legacy_headers.entt;

//import std;

export namespace entt
{

using entt::registry;
using entt::entity;
using entt::null;
using entt::type_id;
using entt::type_info;
using entt::dense_map;
using entt::dense_set;
using entt::operator==;
using entt::operator!=;
using entt::operator<;
using entt::operator<=;
using entt::operator>;
using entt::operator>=;
using entt::dispatcher;
using entt::exclude;
using entt::handle;
using entt::const_handle;
using entt::get;
using entt::hashed_string;

using entt::any;
using entt::basic_any;

using entt::meta_factory;
using entt::resolve;
using entt::meta_handle;
using entt::meta_any;
using entt::meta_range;
using entt::forward_apply;
using entt::forward_as_any;
using entt::forward_as_meta;

using entt::to_integral;
using entt::to_entity;
using entt::to_version;

};

export template<>
struct std::formatter<entt::entity>: std::formatter<int>
{
	template<class FmtContext>
	FmtContext::iterator format(entt::entity id, FmtContext& ctx) const
	{
		return std::format_to(ctx.out(), "{}v{}", entt::to_entity(id), entt::to_version(id));
	}
};
