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
import :utils;

import std;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

export class CpuCommon
{
public:
	CpuCommon(Injector& injector);

	template <typename T, typename data_t = components::FdtdData<T>, typename saver_t = Saver<T>>
	entt::entity init()
	{
		auto id = registry.create();

		components::FdtdDataCreateInfo<T> createInfo {
			.size          = settings.size(),
			.gaussPosition = settings.size()/(std::uint64_t)2,

			//TODO: Get from settings
			.deltaT = (T)1,
			.imp0 = (T)377,
			.Cr = (T)(1.f/std::sqrt(3.f)),

			.maxTime = settings.time(),
			.gaussSigma = 10,
		};

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

#ifndef NDEBUG
		for(auto&& [name, mat]: data.chZippedFields())
			debugPrintSlice(name, mat, data.size);
#endif


		return id;
	}

	template <typename T, typename data_t = components::FdtdData<T>, typename F>
	bool step(entt::entity id, F&& f)
	{
		data_t& data = registry.get<data_t>(id);

		bool canContinue = data.step();

		if(canContinue)
		{
			std::println("Step #{}", data.getTime());

			f(data);

#ifndef NDEBUG
			for(auto&& [name, mat]: data.zippedFields())
				debugPrintSlice(name, mat, data.size);
#endif
		}

		return canContinue;
	}

	template <typename T, typename data_t = components::FdtdData<T>, typename saver_t = Saver<T>>
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
