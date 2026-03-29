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

export module lucuma.systems:simulation_inspector;

import lucuma.services.basic;
import lucuma.services.backends;
import lucuma.events;
import lucuma.utils;
import lucuma.utils.imgui;
import lucuma.components;

import :base;

import imgui;
import std;

namespace lucuma::systems
{

using namespace lucuma::utils;
using namespace services::basic;
using namespace services::backends;

export template<template <typename> class _data, Precision precision>
class SimulationInspector: public Base<SimulationInspector<_data, precision>>
{
public:
	using T      = PrecisionTraits<precision>::type;
	using data_t = _data<T>;
	using base_t = Base<SimulationInspector<_data, precision>>;

	SimulationInspector(Systems& _systems):
		base_t(_systems),
		registry(_systems.inject<entt::registry>()),
		settings(_systems.inject<Settings>())
	{
		init();
	}

	void update([[maybe_unused]] const events::Update& event)
	{
	}

private:
	entt::registry& registry;
	Settings&       settings;

	void init()
	{
	}


};

}
