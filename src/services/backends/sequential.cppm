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
import lucuma.legacy_headers.mdspan;
import lucuma.legacy_headers.entt;

import :base;
import :sequential_fdtd_data;

import std;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

class SequentialBase
{
protected:
	SequentialBase(Injector& injector);

	basic::Settings& settings;

};

export template<Precision precision>
class Sequential: public Base, public SequentialBase
{
public:
	using T = PrecisionTraits<precision>::type;

	using data_t = FdtdData<T>;

	using extents_2d_t = Kokkos::dextents<std::size_t, 2>;
	using extents_3d_t = Kokkos::dextents<std::size_t, 3>;

	using mdspan_2d_t = Kokkos::mdspan<T, extents_2d_t>;
	using mdspan_3d_t = Kokkos::mdspan<T, extents_3d_t>;

	using cmdspan_2d_t = Kokkos::mdspan<const T, extents_2d_t>;
	using cmdspan_3d_t = Kokkos::mdspan<const T, extents_3d_t>;

	Sequential(Injector& injector):
		SequentialBase(injector)
	{ }

	virtual entt::entity init()
	{
		auto id = _registry.create();

		FdtdDataCreateInfo<T> createInfo {
			.size    = settings.size(),

			//TODO: Get from settings
			.deltaT = (T)1,
			.imp0 = (T)377,
			.Cr = (T)(1.f/std::sqrt(3.f)),

			.maxTime = settings.time(),
		};

		data_t& data = _registry.emplace<data_t>(id, createInfo);

		data.initCoefs();

		return id;
	}

	virtual bool step(entt::entity id)
	{
		data_t& data = _registry.get<data_t>(id);

		data.updateH();
		data.updateE();

		return data.step();
	}

	virtual void saveFiles(entt::entity id)
	{
		data_t& data = _registry.get<data_t>(id);

		//TODO
		debugPrint(data.Hx());
	}

	virtual ~Sequential() = default;
private:

	void debugPrint(cmdspan_3d_t mat)
	{
		for(std::size_t i = 0; i < mat.extent(0); i++)
		{
			for(std::size_t j = 0; j < mat.extent(1); j++)
			{
				for(std::size_t k = 0; k < mat.extent(2); k++)
				{
					if constexpr(std::is_default_constructible_v<std::formatter<T>>)
						std::print("{} ", mat[i,j,k]);
					else
						std::print("{} ", (float)mat[i,j,k]);
				}
				std::println();
			}
			std::println();
		}

		std::println("{}", entt::type_id<typeof(mat)>().name());
	}

};

}
