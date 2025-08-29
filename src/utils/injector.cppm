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

#include <entt/entt.hpp>

export module fdtd.utils:injector;

export class Injector
{
public:
	template<typename Type, typename... Args>
	Type& emplace_injectable(Args &&...args) {
		return registry.ctx().emplace<Type>(std::forward<Args>(args)...);
	}

	template<typename Type>
	[[nodiscard]] Type& inject() {
		if(registry.ctx().contains<Type>())
			return registry.ctx().get<Type>();

		return emplace_injectable<Type>(*this);
	}

	template<typename Type>
	[[nodiscard]] const Type& inject() const {
		return registry.ctx().get<Type>();
	}

private:
	entt::registry registry;
};
