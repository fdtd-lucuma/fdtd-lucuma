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

#include <cstdlib>

module lucuma.services.backends;

import std;
import magic_enum;

import :base;
import :cpu_taskflow;
import :sequential;
import :vulkan;

namespace lucuma::utils
{
using namespace lucuma::services::backends;

template<>
struct BackendTraits<Backend::sequential>
{
	template<Precision p>
	using type = Sequential<p>;
};

template<>
struct BackendTraits<Backend::taskflow>
{
	template<Precision p>
	using type = CpuTaskflow<p>;
};

template<>
struct BackendTraits<Backend::vulkan>
{
	template<Precision p>
	using type = Vulkan<p>;
};

}


namespace lucuma::services::backends
{

template <class AlwaysVoid, template <auto> class Template, auto Arg>
struct is_instantiable_helper : std::false_type {};

template <template <auto> class Template, auto Arg>
struct is_instantiable_helper<std::void_t<Template<Arg>>, Template, Arg>
	: std::true_type {};

template <template <auto> class Template, auto Arg>
inline constexpr bool is_instantiable_v =
	is_instantiable_helper<void, Template, Arg>::value;

using namespace lucuma::utils;

template<Backend backend, Precision precision>
constexpr bool isInstantiable()
{
	return is_instantiable_v<BackendTraits<backend>::template type, precision>;
}

constexpr bool isInstantiable(Backend backend, Precision precision)
{
	bool result;
	magic_enum::enum_switch([&](auto precision)
	{
		magic_enum::enum_switch([&](auto backend)
		{
			result = isInstantiable<backend, precision>();

		}, backend);
	}, precision);

	return result;
}

using namespace utils;

Instantiator::Instantiator(Injector& injector):
	injector(injector),
	settings(injector.inject<basic::Settings>())
{ }

IBackend& Instantiator::instantiate()
{
	IBackend* ptr = nullptr;

	magic_enum::enum_switch([&](auto precision)
	{
		magic_enum::enum_switch([&](auto backend)
		{
			if constexpr(isInstantiable(backend, precision))
			{
				using backend_t = typename BackendTraits<backend>::template type<precision>;

				ptr = &injector.emplace<backend_t, backends::IBackend>(injector);
			}
			else
			{
				std::println(std::cerr, "The {} backend doesn't support precision={}.", (Backend)backend, (Precision)precision);
				exit(EXIT_FAILURE);
			}

		}, settings.backend());
	}, settings.precision());

	return *ptr;
}

void Instantiator::instantiateAll()
{
	// TODO: Map which where instantiated
	magic_enum::enum_for_each<Precision>([&](auto precision)
	{
		magic_enum::enum_for_each<Backend>([&](auto backend)
		{
			if constexpr(isInstantiable(backend, precision))
			{
				using backend_t = typename BackendTraits<backend>::template type<precision>;

				injector.emplace<backend_t>(injector);
			}
		});
	});

}

}
