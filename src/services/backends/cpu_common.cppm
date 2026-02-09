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

export module lucuma.services.backends:cpu_common;

import lucuma.utils;
import lucuma.services.basic;
import lucuma.legacy_headers.entt;
import lucuma.components;

import :saver;

import std;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

export class CpuCommon
{
public:
	CpuCommon(Injector& injector);

	template <typename T, typename data_t = components::FdtdData<T>, typename saver_t = Saver<data_t>>
	void init(const components::FdtdDataCreateInfo& createInfo, entt::entity id)
	{
		SaverCreateInfo saverCreateInfo {
			.basePath = ".",
		};

		data_t& data = registry.emplace<data_t>(id, createInfo);

		data.initCoefs();

		if(settings.saveAs() != SaveAs::none)
		{
			saver_t& saver = registry.emplace<saver_t>(id, saverCreateInfo);
			saver.start(data);
		}

		if(settings.debug())
		{
			for(auto&& [name, mat]: data.chZippedFields())
				debugPrintSlice(name, mat, data.size);
			for(auto&& [name, mat]: data.ceZippedFields())
				debugPrintSlice(name, mat, data.size);
		}
	}

	template <typename T, typename data_t = components::FdtdData<T>, typename F>
	bool step(entt::entity id, F&& f)
	{
		if(!registry.valid(id))
			return false;

		data_t& data = registry.get<data_t>(id);

		bool canContinue = data.step();

		if(canContinue)
		{
			if(settings.debug())
				std::println("Step #{}", data.getTime());


			f(data);

			if(settings.debug())
			{
				for(auto&& [name, mat]: data.zippedFields())
					debugPrintSlice(name, mat, data.size);
			}

		}

		return canContinue;
	}

	template <typename T, typename data_t = components::FdtdData<T>, typename saver_t = Saver<data_t>>
	void saveFiles(entt::entity id) //TODO: Move this out of backend
	{
		if(settings.saveAs() == SaveAs::none)
			return;

		auto [data, saver] = registry.get<data_t, saver_t>(id);

		saver.snapshot(data);
	}

private:
	basic::Settings& settings;
	entt::registry& registry;

};

}
