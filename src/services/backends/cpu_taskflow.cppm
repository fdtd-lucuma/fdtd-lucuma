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

#include <tracy/Tracy.hpp>

export module lucuma.services.backends:cpu_taskflow;

import lucuma.utils;
import lucuma.services.basic;
import lucuma.components;
import lucuma.legacy_headers.taskflow;

import :base;
import :cpu_common;

import std;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

class CpuTaskflowBase
{
protected:
	CpuTaskflowBase(Injector& injector);

	CpuCommon& common;

	void initCommon(const components::FdtdDataCreateInfo& createInfo, entt::entity id);

};

export template<Precision precision>
class CpuTaskflow: public IBackend, public CpuTaskflowBase
{
public:
	using T = PrecisionTraits<precision>::type;

	using data_t = components::FdtdData<T>;

	CpuTaskflow(Injector& injector):
		CpuTaskflowBase(injector)
	{ }

	virtual void init(const components::FdtdDataCreateInfo& createInfo, entt::entity id)
	{
		initCommon(createInfo, id);
		common.init<T>(createInfo, id);
	}

	virtual bool step(entt::entity id)
	{
		return common.step<T>(id, [this](data_t& data)
		{
			static tf::Executor executor(3); //TODO Inject this

			tf::Taskflow taskflow;

			auto updateH = taskflow.emplace([&](tf::Subflow& subflow)
			{
				ZoneNamedN(__zone, "H", common.tracy());
				subflow.emplace(
					[&]()
					{
						ZoneNamedN(__zone2, "Hx", common.tracy());
						data.updateHx();
					},
					[&]()
					{
						ZoneNamedN(__zone2, "Hy", common.tracy());
						data.updateHy();
					},
					[&]()
					{
						ZoneNamedN(__zone2, "Hz", common.tracy());
						data.updateHz();
					}
				);
			});

			auto updateE = taskflow.emplace([&](tf::Subflow& subflow)
			{
				ZoneNamedN(__zone, "E", common.tracy());
				subflow.emplace(
					[&]()
					{
						ZoneNamedN(__zone2, "Ex", common.tracy());
						data.updateEx();
					},
					[&]()
					{
						ZoneNamedN(__zone2, "Ey", common.tracy());
						data.updateEy();
					},
					[&]()
					{
						ZoneNamedN(__zone2, "Ez", common.tracy());
						data.updateEz();
					}
				);
			});

			auto gauss = taskflow.emplace([&]()
			{
				ZoneNamedN(__zone, "gauss", common.tracy());
				data.gauss();
			});
			auto abc = taskflow.emplace([&]()
			{
				ZoneNamedN(__zone, "abc", common.tracy());
				data.abc();
			});

			updateH.precede(updateE);
			updateE.precede(gauss);
			gauss.precede(abc);

			updateH.name("H");
			updateE.name("E");
			gauss.name("gauss");
			abc.name("abc");

			executor.run(taskflow).wait();
		});
	}

	virtual void saveFiles(entt::entity id) //TODO: Move this out of backend
	{
		common.saveFiles<T>(id);
	}

	virtual ~CpuTaskflow() = default;
private:

};

// Add one line for each new precision
extern template class CpuTaskflow<Precision::f16>;
extern template class CpuTaskflow<Precision::f32>;
extern template class CpuTaskflow<Precision::f64>;

}
