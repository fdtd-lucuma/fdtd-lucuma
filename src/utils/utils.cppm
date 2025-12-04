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

export module lucuma.utils;

export import :alias;
export import :backend;
export import :copy;
export import :dims;
export import :exceptions;
export import :field;
export import :imgui_graphnode_edge_writer;
export import :injector;
export import :mdspan;
export import :plane;
export import :precision;
export import :print;
export import :save_as;
export import :stream_edge_writer;
export import :vector_component;

import magic_enum;

namespace lucuma::utils
{

template <typename T>
requires std::is_enum_v<T>
struct MagicInstantiator
{
	constexpr static auto values = magic_enum::enum_values<T>();
	constexpr static auto names  = magic_enum::enum_names<T>();
};

extern template struct MagicInstantiator<Backend>;
extern template struct MagicInstantiator<Dim>;
extern template struct MagicInstantiator<Field>;
extern template struct MagicInstantiator<Plane>;
extern template struct MagicInstantiator<Precision>;
extern template struct MagicInstantiator<SaveAs>;
extern template struct MagicInstantiator<VectorComponent>;

}
