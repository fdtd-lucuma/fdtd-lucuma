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

module lucuma.components;

import lucuma.utils;

import :fdtd_data;

// Explicit template instantiations for faster compilation
namespace lucuma::components
{

template class FdtdData<PrecisionTraits<Precision::f16>::type>;
template class FdtdData<PrecisionTraits<Precision::f32>::type>;
template class FdtdData<PrecisionTraits<Precision::f64>::type>;

}
