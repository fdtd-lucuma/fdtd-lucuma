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

namespace lucuma::components
{

export template<typename T>
concept HasComponentEditor = requires(T& t, entt::handle h)
{
	componentEditor(t, h);
};

export template <typename T>
inline void componentEditorCaller(void* p, entt::handle handle)
{
	T& data = *((T*)p);

	componentEditor(data, handle); // For some reason modules makes this fail when it's not on the same namespace as the components.
}

};

namespace lucuma::utils
{


export template <typename T>
void registerComponentEditor()
{
	static_assert(components::HasComponentEditor<T>, "Missing lucuma::components::componentEditor<T> specialization for this type");

	entt::meta_factory<T>{}
		. template func<&components::componentEditorCaller<T>>(entt::hashed_string("componentEditor"));
}


export void showComponentEditor(entt::handle handle);

}

