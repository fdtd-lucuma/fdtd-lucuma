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

export module lucuma.utils:mdspan;

import :alias;

import lucuma.legacy_headers.mdspan;

import std;

namespace lucuma::utils
{

template <typename T>
auto zeroToDim(T n)
{
	return std::pair<T,T>((T)0, n);
}

export template <typename T, typename E, typename L, typename A>
requires (Kokkos::mdspan<T,E,L,A>::rank() == 3)
auto unpad(Kokkos::mdspan<T,E,L,A> matrix, svec3 dims)
{
	return Kokkos::submdspan(
		matrix,
		zeroToDim(dims.x),
		zeroToDim(dims.y),
		zeroToDim(dims.z)
	);
}

export template <typename T, typename E, typename L, typename A>
requires (Kokkos::mdspan<T,E,L,A>::rank() == 2)
auto unpad(Kokkos::mdspan<T,E,L,A> matrix, svec2 dims)
{
	return Kokkos::submdspan(
		matrix,
		zeroToDim(dims.x),
		zeroToDim(dims.y)
	);
}

}
