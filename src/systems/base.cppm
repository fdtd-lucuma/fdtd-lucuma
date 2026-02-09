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

export module lucuma.systems:base;

import lucuma.legacy_headers.entt;
import lucuma.services.basic;
import lucuma.events;

import std;

namespace lucuma::systems
{

template <typename T>
constexpr bool hasUpdate = requires (T t, const lucuma::events::Update& u) {t.update(u);};

template <typename T>
constexpr bool hasPostUpdate = requires (T t, const lucuma::events::PostUpdate& p) {t.postUpdate(p);};

template <typename T>
constexpr bool hasStart = requires (T t, const lucuma::events::Start& s) {t.start(s);};

using namespace services::basic;

export template <typename T>
class Base
{
public:
	static constexpr auto in_place_delete = true;

	Base(Systems& _systems):
		systems(_systems)
	{
		init();
	}

	void init()
	{
		if constexpr(hasUpdate<T>)
			systems.connectUpdate(*(T*)this);
		if constexpr(hasPostUpdate<T>)
			systems.connectPostUpdate(*(T*)this);
		if constexpr(hasStart<T>)
			systems.connectStart(*(T*)this);
	}

	virtual ~Base()
	{
		if constexpr(hasUpdate<T>)
			systems.disconnectUpdate(*(T*)this);
		if constexpr(hasPostUpdate<T>)
			systems.disconnectPostUpdate(*(T*)this);
		if constexpr(hasStart<T>)
			systems.disconnectStart(*(T*)this);
	}

	entt::entity getEntity() const
	{
		return entity;
	}

	friend class lucuma::services::basic::Systems;

	void end([[maybe_unused]]const lucuma::events::End& e)
	{
		systems.stopMine<mine>();
	}

protected:
	Systems&     systems;
	entt::entity entity;

	void selfStop()
	{
		systems.stop(getEntity());
	}

	struct mine {};

	/// Entities created by this function will be deleted when the system is deleted.
	entt::entity createEntity()
	{
		return systems.createMine<mine>();
	}

};

}
