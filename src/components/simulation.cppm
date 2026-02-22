// Una GUI para fdtd
// Copyright © 2025-2026 Otreblan
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

export module lucuma.components:simulation;

import lucuma.utils;
import lucuma.legacy_headers.mdspan;

import std;
import glm;

namespace lucuma::components
{

using namespace lucuma::utils;

export struct SimulationInfo
{
	unsigned int timeI = 0;
	const unsigned int maxTime;

	Backend   backend;
	Precision precision;
};

template <typename T>
T normalizeMinfToInf(T x)
{
	return std::tanh(x);
}

template <>
_Float16 normalizeMinfToInf(_Float16 x)
{
	return normalizeMinfToInf<float>(x);
}

template <typename T>
T normalize0ToInf(T x)
{
	return x/((T)1+x);
}

template <typename T>
T magnitude(T x, T y, T z, T multiplier)
{
	return normalize0ToInf(multiplier*glm::length(glm::vec<3,T>(x,y,z)));
}

template <>
_Float16 magnitude(_Float16 x, _Float16 y, _Float16 z, _Float16 multiplier)
{
	return magnitude<float>(x, y, z, multiplier);
}

export template <typename T>
class HeatmapData
{
public:
	HeatmapData() = default;
	HeatmapData(HeatmapData<T>&& other):
		buffer(std::exchange(other.buffer, {})),
		sizeX(std::exchange(other.sizeX, {})),
		sizeY(std::exchange(other.sizeY, {})),
		_colMajor(std::exchange(_colMajor, {}))
	{}

	template <typename T2, typename E, typename L, typename A>
	requires (Kokkos::mdspan<T2,E,L,A>::rank() == 2)
	void fill(Kokkos::mdspan<T2,E,L,A> plane, T multiplier, bool debug)
	{
		if(debug)
			debugPrint("plane", plane);

		if(plane.empty())
		{
			resize(0, 0);
			return;
		}

		updateColMajor(plane);
		resize(plane.extent(0), plane.extent(1));

		std::size_t bi = 0;

		if(!colMajor())
		{
			for(std::size_t i = 0; i < sizeX; i++)
			{
				for(std::size_t j = 0; j < sizeY; j++)
				{
					buffer[bi++] = normalizeMinfToInf(multiplier*plane[i,j]);
				}
			}
		}
		else
		{
			for(std::size_t j = 0; j < sizeY; j++)
			{
				for(std::size_t i = 0; i < sizeX; i++)
				{
					buffer[bi++] = normalizeMinfToInf(multiplier*plane[i,j]);
				}
			}
		}

		if constexpr (std::is_same_v<T, float>)
		{
			if(debug)
				std::println("{}", buffer);
		}
	}

	template <typename T2,
			typename E, typename L, typename A,
			typename E2, typename L2, typename A2,
			typename E3, typename L3, typename A3
		>
	requires (Kokkos::mdspan<T2,E,L,A>::rank() == 2)
	void fill(
			Kokkos::mdspan<T2,E,L,A> xPlane,
			Kokkos::mdspan<T2,E2,L2,A2> yPlane,
			Kokkos::mdspan<T2,E3,L3,A3> zPlane,
			T multiplier,
			bool debug
			)
	{
		if(xPlane.empty() || yPlane.empty() || zPlane.empty())
		{
			resize(0, 0);
			return;
		}


		if(debug)
		{
			debugPrint("xPlane", xPlane);
			debugPrint("yPlane", yPlane);
			debugPrint("zPlane", zPlane);
		}

		updateColMajor(xPlane);
		resize(std::min(std::min(xPlane.extent(0), yPlane.extent(0)), zPlane.extent(0)), std::min(std::min(xPlane.extent(1), yPlane.extent(1)), zPlane.extent(1)));

		std::size_t bi = 0;
		if(!colMajor())
		{
			for(std::size_t i = 0; i < sizeX; i++)
			{
				for(std::size_t j = 0; j < sizeY; j++)
				{
					buffer[bi++] = magnitude(xPlane[i,j], yPlane[i,j], zPlane[i,j], multiplier);
				}
			}
		}
		else
		{
			for(std::size_t j = 0; j < sizeY; j++)
			{
				for(std::size_t i = 0; i < sizeX; i++)
				{
					buffer[bi++] = magnitude(xPlane[i,j], yPlane[i,j], zPlane[i,j], multiplier);
				}
			}
		}

		if constexpr (std::is_same_v<T, float>)
		{
			if(debug)
				std::println("{}", buffer);
		}
	}


	const T* data() const
	{
		return buffer.empty() ? nullptr : buffer.data();
	}

	std::span<const T> getBuffer() const
	{
		return buffer;
	}

	std::size_t getSizeX() const
	{
		return sizeX;
	}

	std::size_t getSizeY() const
	{
		return sizeY;
	}

	bool colMajor() const
	{
		return _colMajor;
	}

private:
	std::vector<T> buffer;
	std::size_t sizeX = 0;
	std::size_t sizeY = 0;
	bool _colMajor;

	template<typename T2, typename E, typename L, typename A>
	requires (Kokkos::mdspan<T2,E,L,A>::rank() == 2)
	void updateColMajor(Kokkos::mdspan<T2,E,L,A> plane) {
		_colMajor = plane.mapping().stride(0) < plane.mapping().stride(1);
	}

	void resize(std::size_t x, std::size_t y)
	{
		sizeX = x;
		sizeY = y;

		buffer.resize(sizeX*sizeY);
	}
};

export struct SimulationPlotInfo
{
	bool            openWindow      = true;
	Field           field           = Field::Electric;
	VectorComponent vectorComponent = VectorComponent::Magnitude;
	Plane           plane           = Plane::XY;
	int             planeIndex      = 0;
	float           multiplier      = 1000;
	int             nThStep         = 1;
};

export struct Paused {};

};
