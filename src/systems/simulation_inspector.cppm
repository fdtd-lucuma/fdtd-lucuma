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
using namespace lucuma::utils::imgui;
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
		for(auto&& [id, inspector, fdtd_data]: registry.view<components::RegistryInspector, data_t>().each())
		{
			if(!inspector.openWindow)
				continue;

			inspectRegistry(id, StackStr("Inspector #{}", id), inspector, fdtd_data);
		}
	}

private:
	entt::registry& registry;
	Settings&       settings;

	void init()
	{
	}

	void markAsInspectable(entt::handle handle)
	{
		handle.emplace_or_replace<components::ComponentInspector>();
	}

	void inspectRegistry(entt::registry& private_registry, components::RegistryInspector& inspector)
	{
		if(ImGui::Button("New"))
		{
			auto _ = private_registry.create();
		}

		if(ImGui::BeginListBox("##Entities", ImVec2(std::numeric_limits<float>::min(), ImGui::GetContentRegionAvail().y)))
		{
			for(auto&& [id]: private_registry.template view<entt::entity>().each())
			{
				bool is_selected = inspector.selected_idx == id;

				if(ImGui::Selectable(StackStr(id), is_selected))
				{
					inspector.selected_idx = id;
					markAsInspectable({private_registry, id});
				}

				if(is_selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndListBox();
		}
	}

	void inspectRegistry(entt::entity id, const char* window_name, components::RegistryInspector& inspector, data_t& data)
	{
		if(ImGui::Begin(window_name, &inspector.openWindow))
		{
			inspectRegistry(data.getRegistry(), inspector);
		}

		ImGui::End();

		inspectComponents(id, data.getRegistry());
	}

	void inspectComponents(entt::entity parent_id, entt::registry& private_registry)
	{
		for(auto&& [id, inspector]: private_registry.template view<components::ComponentInspector>().each())
		{
			if(!inspector.openWindow)
				continue;

			if(ImGui::Begin(StackStr("Inspector #{},{}", parent_id, id), &inspector.openWindow))
			{
				components::showEditor({private_registry, id});
			}

			ImGui::End();
		}
	}


};

}
