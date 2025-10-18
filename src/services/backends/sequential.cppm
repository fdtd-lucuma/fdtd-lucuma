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

export module lucuma.services.backends:sequential;

import lucuma.utils;
import lucuma.services.basic;
import lucuma.components;

import :base;
import :cpu_common;

import std;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

class SequentialBase
{
protected:
	SequentialBase(Injector& injector);

	CpuCommon& common;

};

export template<Precision precision>
class Sequential: public IBackend, public SequentialBase
{
public:
	using T = PrecisionTraits<precision>::type;

	using data_t = components::FdtdData<T>;

	Sequential(Injector& injector):
		SequentialBase(injector)
	{ }

	virtual entt::entity init()
	{
		return common.init<T>();
	}

	virtual bool step(entt::entity id)
	{
		return common.step<T>(id, [](data_t& data)
		{
			data.updateH();
			data.updateE();
			data.gauss();
			data.abc();
		});
	}

	virtual void saveFiles(entt::entity id) //TODO: Move this out of backend
	{
		common.saveFiles<T>(id);
	}

	virtual ~Sequential() = default;
private:

};

// Add one line for each new precision
extern template class Sequential<Precision::f16>;
extern template class Sequential<Precision::f32>;
extern template class Sequential<Precision::f64>;

}
