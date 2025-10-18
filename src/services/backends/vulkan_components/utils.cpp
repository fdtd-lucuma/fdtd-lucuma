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


module lucuma.services.backends.vulkan_components;

import lucuma.utils;
import std;

namespace lucuma::services::backends::vulkan_components
{

using namespace lucuma::utils;

template <typename T>
requires std::is_arithmetic_v<T>
T pad(T size, T workGroupSize)
{
	return (size % workGroupSize) == 0 ? size : (size/workGroupSize + 1)*workGroupSize;
}

svec3 pad(svec3 size, svec3 workGroupSize)
{
	return {
		pad(size.x, workGroupSize.x),
		pad(size.y, workGroupSize.y),
		pad(size.z, workGroupSize.z),
	};
}

svec2 pad(svec2 size, svec2 workGroupSize)
{
	return {
		pad(size.x, workGroupSize.x),
		pad(size.y, workGroupSize.y)
	};
}

}
