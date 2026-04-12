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

#include <cassert>

module lucuma.utils;

import lucuma.legacy_headers.entt;

namespace lucuma::components
{

void newComponentMenu(entt::handle handle)
{
	for(auto&& it: entt::resolve())
	{
		auto& type = it.second;

		if(auto func = type.func(entt::hashed_string("emplaceComponent")); func)
		{
			auto name = type.data(entt::hashed_string("name"));

			assert(name.is_static());

			if(ImGui::MenuItem(((const std::string*)name.get({}).base().data())->c_str()))
			{
				func.invoke({}, handle);
			}
		}
	}
}

void showEditor(entt::handle handle)
{
	for(auto [id, storage]: handle.storage())
	{
		auto type = entt::resolve(storage.type());
		if(auto func = type.func(entt::hashed_string("editor")); func)
		{
			if(void* value = storage.value(handle.entity()))
			{
				func.invoke({}, value, handle);
			}
		}
	}

	static constexpr const char* popupName = "New components";

	if(ImGui::Button("New component"))
	{
		ImGui::OpenPopup(popupName);
	}

	if(ImGui::BeginPopup(popupName))
	{
		newComponentMenu(handle);

		ImGui::EndPopup();
	}

}

}
