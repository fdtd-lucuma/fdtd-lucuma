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
	size++;
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

SpecializationConstants simpleWorkgroupSize(svec3 workGroupSize)
{
	return SpecializationConstants::make(
		0, (std::uint32_t)workGroupSize.x,
		1, (std::uint32_t)workGroupSize.y,
		2, (std::uint32_t)workGroupSize.z
	);
}

SpecializationConstants workgroupSizeWithDeltas(svec3 workGroupSize, svec3 delta1, svec3 delta2)
{
	return SpecializationConstants::make(
		0, (std::uint32_t)workGroupSize.x,
		1, (std::uint32_t)workGroupSize.y,
		2, (std::uint32_t)workGroupSize.z,
		3, (std::int32_t)delta1.x,
		4, (std::int32_t)delta1.y,
		5, (std::int32_t)delta1.z,
		6, (std::int32_t)delta2.x,
		7, (std::int32_t)delta2.y,
		8, (std::int32_t)delta2.z
	);
}

SpecializationConstants workgroupSizeWithDimAndSlices(svec3 workGroupSize, Dim dim, std::size_t sliceIndex, std::ptrdiff_t sliceDelta)
{
	return SpecializationConstants::make(
		0, (std::uint32_t)workGroupSize.x,
		1, (std::uint32_t)workGroupSize.y,
		2, (std::uint32_t)workGroupSize.z,
		3, dim,
		4, (std::int32_t)sliceIndex,
		5, (std::int32_t)(sliceIndex+sliceDelta)
	);
}

svec3 workGroupCount(svec3 paddedDims, svec3 swizzledWorkGroupSize)
{
	return paddedDims.zyx() / swizzledWorkGroupSize;
}

svec3 slice(svec3 workGroupSize, Dim dim)
{
	// The workgroupSize is swizzled
	switch(dim)
	{
		case Dim::X:
			workGroupSize.z = 1;
			break;
		case Dim::Y:
			workGroupSize.y = 1;
			break;
		case Dim::Z:
			workGroupSize.x = 1;
			break;
	}

	return workGroupSize;
}

std::size_t countOnes(svec3 vec)
{
	std::size_t result = 0;

	if(vec.x == 1)
		result++;
	if(vec.y == 1)
		result++;
	if(vec.z == 1)
		result++;

	return result;
}

}
