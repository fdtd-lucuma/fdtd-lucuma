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

export class Injector
{
public:
	template<typename Type, typename BaseType = Type, typename... Args>
	Type& emplace(Args &&...args) {
		return *registry.ctx().emplace<std::unique_ptr<BaseType>>(std::make_unique<Type>(std::forward<Args>(args)...));
	}

	template<typename Type, typename BaseType = Type>
	[[nodiscard]] BaseType& inject() {
		if(registry.ctx().contains<std::unique_ptr<BaseType>>())
			return *registry.ctx().get<std::unique_ptr<BaseType>>();

		return emplace<Type, BaseType>(*this);
	}

private:
	entt::registry registry;
};
