// Una GUI para fdtd
// Copyright © 2025 Otreblan
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

export module lucuma.utils:injector;

import std;
import lucuma.legacy_headers.entt;

namespace lucuma::utils
{


export class Injector
{
public:
	template<typename Type, typename BaseType = Type, typename... Args>
	requires std::constructible_from<Type, Args...> && std::is_base_of_v<BaseType, Type>
	Type& emplace(Args &&...args)
	{
		// Find out what to do when emplacing the same service twice
		auto watcher = preLink(entt::type_id<BaseType>());
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
		auto watcher = preLink(entt::type_id<BaseType>());
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

			static const std::string msg = "Tried to use "s + entt::type_id<Type>().name() + " but it was not initialized\n";
			std::cerr << msg << '\n';
			throw new std::runtime_error(msg);
		}
	}

	~Injector();

	/// Prints a directed acyclic graph in dot format
	void printEdges(std::ostream& os, const std::string_view removePrefix = "") const;

	void printEdges(const std::filesystem::path& path, const std::string_view removePrefix = "") const;

private:
	class LinkerWatcher
	{
	private:
		Injector& injector;
		bool      linked = false;

	public:
		LinkerWatcher(Injector& injector, entt::type_info type);
		~LinkerWatcher();
	};

	entt::registry registry;

	std::stack<entt::type_info>                              dependencies;
	std::vector<std::pair<entt::type_info, entt::type_info>> dependenciesEdges;

	std::vector<std::function<void()>> deleters;

	LinkerWatcher preLink(entt::type_info type);

	friend class LinkerWatcher;

	template<typename Type, typename BaseType>
	requires std::is_base_of_v<BaseType, Type>
	void onCreate()
	{
		const auto typeId = entt::type_id<Type>();

		// Ensure correct destructor order
		// TODO: Remove this lambda
		deleters.emplace_back([=, this](){
			registry.ctx().erase<std::unique_ptr<BaseType>>();
			std::cerr << "Deleted " << typeId.name() << '\n';
		});

		std::cerr << "Created " << typeId.name() << '\n';
	}
};

}
