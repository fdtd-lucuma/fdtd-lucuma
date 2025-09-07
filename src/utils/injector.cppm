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

export module fdtd.utils:injector;

import std;
import fdtd.legacy_headers.entt;

namespace fdtd::utils
{

export class Injector
{
public:
	template<typename Type, typename BaseType = Type, typename... Args>
	requires std::constructible_from<Type, Args...> && std::is_base_of_v<BaseType, Type>
	Type& emplace(Args &&...args)
	{
		auto ptr = std::make_unique<Type>(std::forward<Args>(args)...);
		auto& ref = *ptr;

		onCreate<Type, BaseType>();

		registry.ctx().emplace<std::unique_ptr<BaseType>>(std::move(ptr));
		return ref;
	}

	template<typename Type, typename BaseType = Type>
	requires std::is_base_of_v<BaseType, Type>
	[[nodiscard]] BaseType& inject()
	{
		auto ptr = registry.ctx().find<std::unique_ptr<BaseType>>();

		if(ptr != nullptr)
			return **ptr;

		if constexpr(std::constructible_from<Type, Injector&>)
		{
			return emplace<Type, BaseType>(*this);
		}
		else
		{
			using namespace std::literals::string_literals;

			static const std::string msg = "Tried to use "s + typeid(Type).name() + " but it was not initialized\n";
			std::cerr << msg << '\n';
			throw new std::runtime_error(msg);
		}
	}

	~Injector();

private:
	entt::registry registry;

	std::vector<std::function<void()>> deleters;

	template<typename Type, typename BaseType>
	requires std::is_base_of_v<BaseType, Type>
	void onCreate()
	{
		// Ensure correct destructor order
		deleters.emplace_back([this](){
			registry.ctx().erase<std::unique_ptr<BaseType>>();
			std::cerr << "Deleted " << typeid(Type).name() << '\n';
		});

		std::cerr << "Created " << typeid(Type).name() << '\n';
	}
};

}
