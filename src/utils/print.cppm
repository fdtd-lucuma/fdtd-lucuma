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

export module lucuma.utils:print;

import std.compat;

import lucuma.legacy_headers.entt;
import lucuma.legacy_headers.fmt;
import lucuma.legacy_headers.mdspan;

import :alias;

namespace lucuma::utils
{

export template <typename... Args>
void printAll(std::ostream& os, Args&&... args) {
	(std::println(os, "{}", std::forward<Args>(args)), ...);
}

export template <typename F>
void writeToFile(const std::filesystem::path& path, F&& f)
{
	auto ofs = std::ofstream(path);

	if(!ofs.is_open())
	{
		perror(path.c_str());
		return;
	}

	// Why is locale so slow?
	ofs.imbue(std::locale::classic());
	f(ofs);

}

export template <typename F>
void fastWriteToFile(const std::filesystem::path& path, F&& f)
{
	auto out = fmt::output_file(path.c_str());

	// TODO: Check if fmt::ostream is open

	f(out);

}

export template <typename T>
inline auto toPrintable(T x)
{
	if constexpr(std::is_default_constructible_v<std::formatter<T>>)
		return x;
	else
		return (float)x;
}

#ifndef NDEBUG

export template<typename T, typename E, typename L, typename A>
void debugPrint(Kokkos::mdspan<T,E,L,A> mat)
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

export template<typename T, typename E, typename L, typename A>
void debugPrintSlice(std::string_view name, Kokkos::mdspan<T,E,L,A> _mat, svec3 size)
{
	auto sliceIndex = size.x/2;
	auto mat = Kokkos::submdspan(_mat, sliceIndex, Kokkos::full_extent, Kokkos::full_extent);

	std::println("{} slice x={}", name, sliceIndex);

	for(std::size_t i = 0; i < mat.extent(0); i++)
	{
		for(std::size_t j = 0; j < mat.extent(1); j++)
		{
			std::print("{} ", toPrintable(mat[i,j]));
		}
		std::println();
	}

	std::println("{}", entt::type_id<typeof(mat)>().name());
}

export template <typename T, typename E, typename L, typename A>
void debugPrint(std::string_view name, Kokkos::mdspan<T, E, L, A> mat)
{
	static_assert(mat.rank() == 2);

	std::println("{}: {}, size = {},{}", name, entt::type_id<typeof(mat)>().name(), mat.extent(0), mat.extent(1));

}

#endif



}
