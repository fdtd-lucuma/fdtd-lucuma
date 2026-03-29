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
import imgui;

namespace lucuma::components
{

export template<typename T>
concept HasEditor = requires(T& t, entt::handle h)
{
	editor(t, h);
};

export template <HasEditor T>
void registerEditor()
{
	static const std::string name(entt::type_id<T>().name());

	entt::meta_factory<T>{}
		.template func<+[](void* p, entt::handle handle)
		{
			bool visible = true;

			if(ImGui::CollapsingHeader(name.c_str(), &visible, ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushID(name.c_str());
				editor(*(T*)p, handle); // For some reason modules makes this fail when it's not on the same namespace as the components.
				ImGui::PopID();

			}

			if(!visible)
				handle.erase<T>();
		}>(entt::hashed_string("editor"))
		.template func<+[](entt::handle handle)
		{
			handle.emplace_or_replace<T>();
		}>(entt::hashed_string("emplaceComponent"))
		.template data<&name>(entt::hashed_string("name"));
}


export void showEditor(entt::handle handle);

}

