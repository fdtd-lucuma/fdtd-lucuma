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
import :saver;
import :utils;

import std;
import glm;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

class SequentialBase
{
protected:
	SequentialBase(Injector& injector);

	basic::Settings& settings;
	entt::registry& registry;

};

export template<Precision precision>
class Sequential: public IBackend, public SequentialBase
{
public:
	using T = PrecisionTraits<precision>::type;

	using data_t  = FdtdData<T>;
	using saver_t = Saver<T>;

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
		auto id = registry.create();

		FdtdDataCreateInfo<T> createInfo {
			.size          = settings.size(),
			.gaussPosition = settings.size()/(std::ptrdiff_t)2,

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

		data_t& data   = registry.emplace<data_t>(id, createInfo);
		saver_t& saver = registry.emplace<saver_t>(id, saverCreateInfo);

		data.initCoefs();
		saver.start(data);


		return id;
	}

	virtual bool step(entt::entity id)
	{
		data_t& data = registry.get<data_t>(id);

		bool canContinue = data.step();

		if(canContinue)
		{
			std::println("Step #{}", data.getTime());

			data.updateH();
			data.updateE();
			data.gauss();
			data.abc();
		}

		return canContinue;
	}

	virtual void saveFiles(entt::entity id)
	{
		auto [data, saver] = registry.get<data_t, saver_t>(id);

		saver.snapshot(data);

#ifndef NDEBUG
		//debugPrint(data.Ex());
#endif
	}

	virtual ~Sequential() = default;
private:

#ifndef NDEBUG
	void debugPrint(cmdspan_3d_t mat)
	{
		for(std::size_t i = 0; i < mat.extent(0); i++)
		{
			for(std::size_t j = 0; j < mat.extent(1); j++)
			{
				for(std::size_t k = 0; k < mat.extent(2); k++)
				{
					std::print("{:.2f} ", toPrintable(mat[i,j,k]));
				}
				std::println();
			}
			std::println();
		}

		std::println("{}", entt::type_id<typeof(mat)>().name());
	}
#endif

};

// Add one line for each new precision
extern template class Sequential<Precision::f16>;
extern template class Sequential<Precision::f32>;
extern template class Sequential<Precision::f64>;

}
