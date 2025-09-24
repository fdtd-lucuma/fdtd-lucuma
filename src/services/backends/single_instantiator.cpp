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
import :sequential;
import :vulkan;
import :utils;

namespace lucuma::services::backends
{

using namespace utils;

SingleInstantiator::SingleInstantiator(Injector& injector):
	settings(injector.inject<basic::Settings>()),
	instance(instantiate(injector))
{ }

Base& SingleInstantiator::instantiate(Injector& injector)
{
	Base* ptr = nullptr;

	magic_enum::enum_switch([&](auto precision)
	{
		magic_enum::enum_switch([&](auto backend)
		{
			if constexpr(isInstantiable(backend, precision))
			{
				using backend_t = typename BackendTraits<backend>::template type<precision>;

				ptr = &injector.emplace<backend_t, backends::Base>(injector);
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

}
