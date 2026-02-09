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

export module lucuma.services.basic:systems;

import lucuma.utils;
import std;
import lucuma.legacy_headers.entt;
import lucuma.events;

namespace lucuma::services::basic
{

struct OnSystemEnd
{
	std::function<void(entt::registry&, entt::entity)> f;
};

template <typename T>
constexpr bool hasEnd = requires (T t, const lucuma::events::End& e) {t.end(e);};

using namespace lucuma::utils;

export class Systems
{
public:
	Systems(Injector& _injector);

	template <typename T, typename... Args>
	requires std::is_constructible_v<T, Systems&, Args...>
	entt::entity start(Args &&...args)
	{
		entt::entity e = createEntity();

		T& s = registry.emplace<T>(e, *this, std::forward<Args>(args)...);
		s.entity = e;

		if constexpr(hasEnd<T>)
		{
			registry.emplace<OnSystemEnd>(e, [](entt::registry& registry, entt::entity e){
				registry.get<T>(e).end({});
			});
		}

		return e;
	}

	template<typename Type, typename BaseType = Type>
	requires std::is_base_of_v<BaseType, Type>
	[[nodiscard]] BaseType& inject()
	{
		return injector.inject<Type, BaseType>();
	}

	template <typename T>
	void connectUpdate(T& system)
	{
		dispatcher.sink<events::Update>().connect<&T::update>(system);
	}

	template <typename T>
	void connectPostUpdate(T& system)
	{
		dispatcher.sink<events::PostUpdate>().connect<&T::postUpdate>(system);
	}

	template <typename T>
	void connectStart(T& system)
	{
		dispatcher.sink<events::Start>().connect<&T::start>(system);
	}

	template <typename T>
	void disconnectUpdate(T& system)
	{
		dispatcher.sink<events::Update>().disconnect<&T::update>(system);
	}

	template <typename T>
	void disconnectPostUpdate(T& system)
	{
		dispatcher.sink<events::PostUpdate>().disconnect<&T::postUpdate>(system);
	}

	template <typename T>
	void disconnectStart(T& system)
	{
		dispatcher.sink<events::Start>().disconnect<&T::start>(system);
	}

	void stop(entt::entity e);

	template<typename It>
	void stop(It first, It last)
	{
		registry.insert<toStop>(first, last);
	}

	template<typename T>
	void stopMine()
	{
		auto view = registry.view<T>();
		registry.insert<toStop>(view.begin(), view.end());
	}

	template<typename T>
	entt::entity createMine()
	{
		entt::entity e = registry.create();

		registry.emplace<T>(e);

		return e;
	}

	void cleanStopped();

	~Systems();

private:
	entt::dispatcher& dispatcher;
	entt::registry&   registry;
	Injector&         injector;

	entt::entity createEntity();

	struct toStop {};
	struct mine {};

};

}
