// Una GUI para fdtd
// Copyright © 2025 Otreblan
//
// fdtd-vulkan is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// fdtd-vulkan is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fdtd-vulkan.  If not, see <http://www.gnu.org/licenses/>.

module;

#include <entt/entity/registry.hpp>
#include <iostream>

export module fdtd.utils:injector;

template<typename T>
constexpr bool is_move_only_v = std::is_move_constructible_v<T> && !std::is_copy_constructible_v<T>;

export class Injector
{
public:
	template<typename Type, typename... Args>
	requires is_move_only_v<Type>
	Type& emplace(Args &&...args) {
		Type newObject(std::forward<Args>(args)...);

		return registry.ctx().emplace<Type>(std::move(newObject));
	}

	template<typename Type>
	requires is_move_only_v<Type>
	[[nodiscard]] Type& inject() {
		if(registry.ctx().contains<Type>())
			return registry.ctx().get<Type>();

		return emplace<Type>(*this);
	}

private:
	entt::registry registry;
};
