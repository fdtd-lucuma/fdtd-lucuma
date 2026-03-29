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

export module lucuma.utils:meta;

import lucuma.legacy_headers.entt;
import std;

namespace lucuma::editor
{

export template <typename T>
void componentEditor(entt::handle)
{
	std::println("{} {}", "It works", entt::type_id<T>().name());
}

};

namespace lucuma::utils
{

export template <typename T>
void registerComponentEditor()
{
	entt::meta_factory<T>{}
		. template func<&lucuma::editor::componentEditor<T>>(entt::hashed_string("componentEditor"));
}


export void showComponentEditor(entt::handle handle);

}

