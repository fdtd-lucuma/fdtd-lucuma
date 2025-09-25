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

#include <cassert>

export module lucuma.services.backends:sequential_fdtd_data;

import lucuma.utils;
import lucuma.legacy_headers.mdspan;
import lucuma.legacy_headers.entt;

import std;
import glm;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

template <typename T>
//requires std::is_arithmetic_v<T>
std::vector<T> initMat(svec3 dims, T defaultValue = 0)
{
	return std::vector<T>(dims.x*dims.y*dims.z, defaultValue);
}

template <typename T>
//requires std::is_arithmetic_v<T>
std::vector<T> initMat(svec2 dims, T defaultValue = 0)
{
	return std::vector<T>(dims.x*dims.y, defaultValue);
}

template <class T>
struct FdtdDataCreateInfo
{
	svec3 size;
	svec3 gaussPosition;
	T deltaT;
	T imp0;
	T Cr;
	unsigned int maxTime;
	T gaussSigma;
};

template <class T>
class FdtdData
{
public:
	constexpr static auto HxDimsDelta = svec3(0, -1, -1);
	constexpr static auto HyDimsDelta = svec3(-1, 0, -1);
	constexpr static auto HzDimsDelta = svec3(-1, -1, 0);

	constexpr static auto ExDimsDelta = svec3(-1, 0, 0);
	constexpr static auto EyDimsDelta = svec3(0, -1, 0);
	constexpr static auto EzDimsDelta = svec3(0, 0, -1);

	template <typename extents, typename layout = Kokkos::layout_right>
	using mdspan_t = Kokkos::mdspan<T, extents, layout>;

	template <typename extents, typename layout = Kokkos::layout_right>
	using cmdspan_t = Kokkos::mdspan<const T, extents, layout>;

	using extents_2d_t = Kokkos::dextents<std::size_t, 2>;
	using extents_3d_t = Kokkos::dextents<std::size_t, 3>;

	template <typename layout = Kokkos::layout_right>
	using _mdspan_2d_t = mdspan_t<extents_2d_t, layout>;
	template <typename layout = Kokkos::layout_right>
	using _mdspan_3d_t = mdspan_t<extents_3d_t, layout>;

	template <typename layout = Kokkos::layout_right>
	using _cmdspan_2d_t = cmdspan_t<extents_2d_t, layout>;
	template <typename layout = Kokkos::layout_right>
	using _cmdspan_3d_t = cmdspan_t<extents_3d_t, layout>;

	using mdspan_2d_t = _mdspan_2d_t<>;
	using mdspan_3d_t = _mdspan_3d_t<>;

	using cmdspan_2d_t = _cmdspan_2d_t<>;
	using cmdspan_3d_t = _cmdspan_3d_t<>;

private:

	static inline mdspan_3d_t toMdspan(std::vector<T>& v, svec3 dims)
	{
		return mdspan_3d_t(v.data(), dims.x, dims.y, dims.z);
	}

	static inline mdspan_2d_t toMdspan(std::vector<T>& v, svec2 dims)
	{
		return mdspan_2d_t(v.data(), dims.x, dims.y);
	}

	static inline cmdspan_3d_t toMdspan(const std::vector<T>& v, svec3 dims)
	{
		return cmdspan_3d_t(v.data(), dims.x, dims.y, dims.z);
	}

	static inline cmdspan_2d_t toMdspan(const std::vector<T>& v, svec2 dims)
	{
		return cmdspan_2d_t(v.data(), dims.x, dims.y);
	}


public:
	FdtdData(const FdtdDataCreateInfo<T>& createInfo):
		size(createInfo.size),
		gaussPosition(createInfo.gaussPosition),
		deltaT(createInfo.deltaT),
		imp0(createInfo.imp0),
		Cr(createInfo.Cr),
		maxTime(createInfo.maxTime),
		gaussSigma(createInfo.gaussSigma),
		HxDims(size + HxDimsDelta),
		HyDims(size + HyDimsDelta),
		HzDims(size + HzDimsDelta),
		ExDims(size + ExDimsDelta),
		EyDims(size + EyDimsDelta),
		EzDims(size + EzDimsDelta),
		eyxDims(EyDims.yz()),
		ezxDims(EzDims.yz()),
		exyDims(ExDims.xz()),
		ezyDims(EzDims.xz()),
		exzDims(ExDims.xy()),
		eyzDims(EyDims.xy()),
		_Hx(initMat<T>(HxDims)),
		_Hy(initMat<T>(HyDims)),
		_Hz(initMat<T>(HzDims)),
		_Chxh(initMat<T>(HxDims)),
		_Chyh(initMat<T>(HyDims)),
		_Chzh(initMat<T>(HzDims)),
		_Chxe(initMat<T>(HxDims)),
		_Chye(initMat<T>(HyDims)),
		_Chze(initMat<T>(HzDims)),
		_CMhx(initMat<T>(HxDims)),
		_CMhy(initMat<T>(HyDims)),
		_CMhz(initMat<T>(HzDims)),
		_mux(initMat<T>(HxDims, 1)),
		_muy(initMat<T>(HyDims, 1)),
		_muz(initMat<T>(HzDims, 1)),
		_muxR(initMat<T>(size, 1)),
		_muyR(initMat<T>(size, 1)),
		_muzR(initMat<T>(size, 1)),
		_Ex(initMat<T>(ExDims)),
		_Ey(initMat<T>(EyDims)),
		_Ez(initMat<T>(EzDims)),
		_Cexe(initMat<T>(ExDims)),
		_Ceye(initMat<T>(EyDims)),
		_Ceze(initMat<T>(EzDims)),
		_Cexh(initMat<T>(ExDims)),
		_Ceyh(initMat<T>(EyDims)),
		_Cezh(initMat<T>(EzDims)),
		_CEEx(initMat<T>(ExDims)),
		_CEEy(initMat<T>(EyDims)),
		_CEEz(initMat<T>(EzDims)),
		_epsx(initMat<T>(ExDims, 1)),
		_epsy(initMat<T>(EyDims, 1)),
		_epsz(initMat<T>(EzDims, 1)),
		_epsxR(initMat<T>(size, 1)),
		_epsyR(initMat<T>(size, 1)),
		_epszR(initMat<T>(size, 1)),
		_eyx0(initMat<T>(eyxDims)),
		_ezx0(initMat<T>(ezxDims)),
		_eyx1(initMat<T>(eyxDims)),
		_ezx1(initMat<T>(ezxDims)),
		_exy0(initMat<T>(exyDims)),
		_ezy0(initMat<T>(ezyDims)),
		_exy1(initMat<T>(exyDims)),
		_ezy1(initMat<T>(ezyDims)),
		_exz0(initMat<T>(exzDims)),
		_eyz0(initMat<T>(eyzDims)),
		_exz1(initMat<T>(exzDims)),
		_eyz1(initMat<T>(eyzDims))
	{}

	mdspan_3d_t Hx()    { return toMdspan(_Hx,    HxDims);  }
	mdspan_3d_t Hy()    { return toMdspan(_Hy,    HyDims);  }
	mdspan_3d_t Hz()    { return toMdspan(_Hz,    HzDims);  }
	mdspan_3d_t Chxh()  { return toMdspan(_Chxh,  HxDims);  }
	mdspan_3d_t Chyh()  { return toMdspan(_Chyh,  HyDims);  }
	mdspan_3d_t Chzh()  { return toMdspan(_Chzh,  HzDims);  }
	mdspan_3d_t Chxe()  { return toMdspan(_Chxe,  HxDims);  }
	mdspan_3d_t Chye()  { return toMdspan(_Chye,  HyDims);  }
	mdspan_3d_t Chze()  { return toMdspan(_Chze,  HzDims);  }
	mdspan_3d_t CMhx()  { return toMdspan(_CMhx,  HxDims);  }
	mdspan_3d_t CMhy()  { return toMdspan(_CMhy,  HyDims);  }
	mdspan_3d_t CMhz()  { return toMdspan(_CMhz,  HzDims);  }
	mdspan_3d_t mux()   { return toMdspan(_mux,   HxDims);  }
	mdspan_3d_t muy()   { return toMdspan(_muy,   HyDims);  }
	mdspan_3d_t muz()   { return toMdspan(_muz,   HzDims);  }
	mdspan_3d_t muxR()  { return toMdspan(_muxR,  size);    }
	mdspan_3d_t muyR()  { return toMdspan(_muyR,  size);    }
	mdspan_3d_t muzR()  { return toMdspan(_muzR,  size);    }
	mdspan_3d_t Ex()    { return toMdspan(_Ex,    ExDims);  }
	mdspan_3d_t Ey()    { return toMdspan(_Ey,    EyDims);  }
	mdspan_3d_t Ez()    { return toMdspan(_Ez,    EzDims);  }
	mdspan_3d_t Cexe()  { return toMdspan(_Cexe,  ExDims);  }
	mdspan_3d_t Ceye()  { return toMdspan(_Ceye,  EyDims);  }
	mdspan_3d_t Ceze()  { return toMdspan(_Ceze,  EzDims);  }
	mdspan_3d_t Cexh()  { return toMdspan(_Cexh,  ExDims);  }
	mdspan_3d_t Ceyh()  { return toMdspan(_Ceyh,  EyDims);  }
	mdspan_3d_t Cezh()  { return toMdspan(_Cezh,  EzDims);  }
	mdspan_3d_t CEEx()  { return toMdspan(_CEEx,  ExDims);  }
	mdspan_3d_t CEEy()  { return toMdspan(_CEEy,  EyDims);  }
	mdspan_3d_t CEEz()  { return toMdspan(_CEEz,  EzDims);  }
	mdspan_3d_t epsx()  { return toMdspan(_epsx,  ExDims);  }
	mdspan_3d_t epsy()  { return toMdspan(_epsy,  EyDims);  }
	mdspan_3d_t epsz()  { return toMdspan(_epsz,  EzDims);  }
	mdspan_3d_t epsxR() { return toMdspan(_epsxR, size);    }
	mdspan_3d_t epsyR() { return toMdspan(_epsyR, size);    }
	mdspan_3d_t epszR() { return toMdspan(_epszR, size);    }
	mdspan_2d_t eyx0()  { return toMdspan(_eyx0,  eyxDims); }
	mdspan_2d_t ezx0()  { return toMdspan(_ezx0,  ezxDims); }
	mdspan_2d_t eyx1()  { return toMdspan(_eyx1,  eyxDims); }
	mdspan_2d_t ezx1()  { return toMdspan(_ezx1,  ezxDims); }
	mdspan_2d_t exy0()  { return toMdspan(_exy0,  exyDims); }
	mdspan_2d_t ezy0()  { return toMdspan(_ezy0,  ezyDims); }
	mdspan_2d_t exy1()  { return toMdspan(_exy1,  exyDims); }
	mdspan_2d_t ezy1()  { return toMdspan(_ezy1,  ezyDims); }
	mdspan_2d_t exz0()  { return toMdspan(_exz0,  exzDims); }
	mdspan_2d_t eyz0()  { return toMdspan(_eyz0,  eyzDims); }
	mdspan_2d_t exz1()  { return toMdspan(_exz1,  exzDims); }
	mdspan_2d_t eyz1()  { return toMdspan(_eyz1,  eyzDims); }

	cmdspan_3d_t Hx()    const { return toMdspan(_Hx,    HxDims);  }
	cmdspan_3d_t Hy()    const { return toMdspan(_Hy,    HyDims);  }
	cmdspan_3d_t Hz()    const { return toMdspan(_Hz,    HzDims);  }
	cmdspan_3d_t Chxh()  const { return toMdspan(_Chxh,  HxDims);  }
	cmdspan_3d_t Chyh()  const { return toMdspan(_Chyh,  HyDims);  }
	cmdspan_3d_t Chzh()  const { return toMdspan(_Chzh,  HzDims);  }
	cmdspan_3d_t Chxe()  const { return toMdspan(_Chxe,  HxDims);  }
	cmdspan_3d_t Chye()  const { return toMdspan(_Chye,  HyDims);  }
	cmdspan_3d_t Chze()  const { return toMdspan(_Chze,  HzDims);  }
	cmdspan_3d_t CMhx()  const { return toMdspan(_CMhx,  HxDims);  }
	cmdspan_3d_t CMhy()  const { return toMdspan(_CMhy,  HyDims);  }
	cmdspan_3d_t CMhz()  const { return toMdspan(_CMhz,  HzDims);  }
	cmdspan_3d_t mux()   const { return toMdspan(_mux,   HxDims);  }
	cmdspan_3d_t muy()   const { return toMdspan(_muy,   HyDims);  }
	cmdspan_3d_t muz()   const { return toMdspan(_muz,   HzDims);  }
	cmdspan_3d_t muxR()  const { return toMdspan(_muxR,  size);    }
	cmdspan_3d_t muyR()  const { return toMdspan(_muyR,  size);    }
	cmdspan_3d_t muzR()  const { return toMdspan(_muzR,  size);    }
	cmdspan_3d_t Ex()    const { return toMdspan(_Ex,    ExDims);  }
	cmdspan_3d_t Ey()    const { return toMdspan(_Ey,    EyDims);  }
	cmdspan_3d_t Ez()    const { return toMdspan(_Ez,    EzDims);  }
	cmdspan_3d_t Cexe()  const { return toMdspan(_Cexe,  ExDims);  }
	cmdspan_3d_t Ceye()  const { return toMdspan(_Ceye,  EyDims);  }
	cmdspan_3d_t Ceze()  const { return toMdspan(_Ceze,  EzDims);  }
	cmdspan_3d_t Cexh()  const { return toMdspan(_Cexh,  ExDims);  }
	cmdspan_3d_t Ceyh()  const { return toMdspan(_Ceyh,  EyDims);  }
	cmdspan_3d_t Cezh()  const { return toMdspan(_Cezh,  EzDims);  }
	cmdspan_3d_t CEEx()  const { return toMdspan(_CEEx,  ExDims);  }
	cmdspan_3d_t CEEy()  const { return toMdspan(_CEEy,  EyDims);  }
	cmdspan_3d_t CEEz()  const { return toMdspan(_CEEz,  EzDims);  }
	cmdspan_3d_t epsx()  const { return toMdspan(_epsx,  ExDims);  }
	cmdspan_3d_t epsy()  const { return toMdspan(_epsy,  EyDims);  }
	cmdspan_3d_t epsz()  const { return toMdspan(_epsz,  EzDims);  }
	cmdspan_3d_t epsxR() const { return toMdspan(_epsxR, size);    }
	cmdspan_3d_t epsyR() const { return toMdspan(_epsyR, size);    }
	cmdspan_3d_t epszR() const { return toMdspan(_epszR, size);    }
	cmdspan_2d_t eyx0()  const { return toMdspan(_eyx0,  eyxDims); }
	cmdspan_2d_t ezx0()  const { return toMdspan(_ezx0,  ezxDims); }
	cmdspan_2d_t eyx1()  const { return toMdspan(_eyx1,  eyxDims); }
	cmdspan_2d_t ezx1()  const { return toMdspan(_ezx1,  ezxDims); }
	cmdspan_2d_t exy0()  const { return toMdspan(_exy0,  exyDims); }
	cmdspan_2d_t ezy0()  const { return toMdspan(_ezy0,  ezyDims); }
	cmdspan_2d_t exy1()  const { return toMdspan(_exy1,  exyDims); }
	cmdspan_2d_t ezy1()  const { return toMdspan(_ezy1,  ezyDims); }
	cmdspan_2d_t exz0()  const { return toMdspan(_exz0,  exzDims); }
	cmdspan_2d_t eyz0()  const { return toMdspan(_eyz0,  eyzDims); }
	cmdspan_2d_t exz1()  const { return toMdspan(_exz1,  exzDims); }
	cmdspan_2d_t eyz1()  const { return toMdspan(_eyz1,  eyzDims); }

	std::generator<std::tuple<const char*, cmdspan_3d_t>> zippedFields() const {
		static constexpr std::array names {
			"Hx",
			"Hy",
			"Hz",
			"Ex",
			"Ey",
			"Ez",
		};

		std::array mats {
			Hx(),
			Hy(),
			Hz(),
			Ex(),
			Ey(),
			Ez(),
		};

		for(auto&& p: std::views::zip(names, mats))
			co_yield p;
	}

	/// Returns true and increments the counter by +1 if it can still continue.
	bool step() {
		if(time >= maxTime)
			return false;

		time++;
		return true;
	}

	const svec3 size;
	const svec3 gaussPosition;
	const T deltaT;
	const T imp0;
	const T Cr;

	const unsigned int maxTime;

private:
	unsigned int time = 0;
	T gaussSigma;

	// Magnetic field dimentions

	const svec3 HxDims;
	const svec3 HyDims;
	const svec3 HzDims;

	// Electric field dimentions

	const svec3 ExDims;
	const svec3 EyDims;
	const svec3 EzDims;

	// ABC dimentions

	const svec2 eyxDims;
	const svec2 ezxDims;

	const svec2 exyDims;
	const svec2 ezyDims;

	const svec2 exzDims;
	const svec2 eyzDims;

	// Magnetic fields

	std::vector<T> _Hx;
	std::vector<T> _Hy;
	std::vector<T> _Hz;

	std::vector<T> _Chxh;
	std::vector<T> _Chyh;
	std::vector<T> _Chzh;

	std::vector<T> _Chxe;
	std::vector<T> _Chye;
	std::vector<T> _Chze;

	std::vector<T> _CMhx;
	std::vector<T> _CMhy;
	std::vector<T> _CMhz;

	std::vector<T> _mux;
	std::vector<T> _muy;
	std::vector<T> _muz;

	std::vector<T> _muxR;
	std::vector<T> _muyR;
	std::vector<T> _muzR;

	// Electric fields

	std::vector<T> _Ex;
	std::vector<T> _Ey;
	std::vector<T> _Ez;

	std::vector<T> _Cexe;
	std::vector<T> _Ceye;
	std::vector<T> _Ceze;

	std::vector<T> _Cexh;
	std::vector<T> _Ceyh;
	std::vector<T> _Cezh;

	std::vector<T> _CEEx;
	std::vector<T> _CEEy;
	std::vector<T> _CEEz;

	std::vector<T> _epsx;
	std::vector<T> _epsy;
	std::vector<T> _epsz;

	std::vector<T> _epsxR;
	std::vector<T> _epsyR;
	std::vector<T> _epszR;

	// ABC's

	std::vector<T> _eyx0;
	std::vector<T> _ezx0;
	std::vector<T> _eyx1;
	std::vector<T> _ezx1;

	std::vector<T> _exy0;
	std::vector<T> _ezy0;
	std::vector<T> _exy1;
	std::vector<T> _ezy1;

	std::vector<T> _exz0;
	std::vector<T> _eyz0;
	std::vector<T> _exz1;
	std::vector<T> _eyz1;

public:
	unsigned int getTime() const
	{
		return time;
	}

	// Parameters are named for H.
	void initCoef(
		mdspan_3d_t Ch,
		mdspan_3d_t Ce,
		cmdspan_3d_t CM,
		cmdspan_3d_t mu
	)
	{
		assert(Ch.extents() == Ce.extents());
		assert(Ce.extents() == CM.extents());
		assert(CM.extents() == mu.extents());

		const std::size_t x = Ch.extent(0);
		const std::size_t y = Ch.extent(1);
		const std::size_t z = Ch.extent(2);

		#pragma omp parallel for default(private) firstprivate(x,y,z,Cr,imp0,deltaT)
		for(std::size_t i = 0; i < x; i++)
		{
			for(std::size_t j = 0; j < y; j++)
			{
				for(std::size_t k = 0; k < z; k++)
				{
					const T c = (CM[i,j,k]*deltaT)/((T)2*mu[i,j,k]);

					Ch[i,j,k] = ((T)1-c)/((T)1+c);
					Ce[i,j,k] = ((T)1/((T)1+c))*(Cr/imp0);
				}
			}
		}
	}

	void initCoefHx()
	{
		initCoef(
			Chxh(),
			Chxe(),
			CMhx(),
			mux()
		);
	}

	void initCoefHy()
	{
		initCoef(
			Chyh(),
			Chye(),
			CMhy(),
			muy()
		);
	}

	void initCoefHz()
	{
		initCoef(
			Chzh(),
			Chze(),
			CMhz(),
			muz()
		);
	}

	void initCoefEx()
	{
		initCoef(
			Cexe(),
			Cexh(),
			CEEx(),
			epsx()
		);
	}

	void initCoefEy()
	{
		initCoef(
			Ceye(),
			Ceyh(),
			CEEy(),
			epsy()
		);
	}

	void initCoefEz()
	{
		initCoef(
			Ceze(),
			Cezh(),
			CEEz(),
			epsz()
		);
	}

	void initCoefs()
	{
		initCoefHx();
		initCoefHy();
		initCoefHz();
		initCoefEx();
		initCoefEy();
		initCoefEz();
	}

	template<svec3 Ec1Delta, svec3 Ec2Delta>
	void updateHComponent(
		mdspan_3d_t Hc,
		cmdspan_3d_t Ch,
		cmdspan_3d_t Ce,
		cmdspan_3d_t Ec1,
		cmdspan_3d_t Ec2
	)
	{
		const std::size_t x = Hc.extent(0);
		const std::size_t y = Hc.extent(1);
		const std::size_t z = Hc.extent(2);

		assert(x-1+Ec1Delta.x < Ec1.extent(0));
		assert(y-1+Ec1Delta.y < Ec1.extent(1));
		assert(z-1+Ec1Delta.z < Ec1.extent(2));

		assert(x-1+Ec2Delta.x < Ec2.extent(0));
		assert(y-1+Ec2Delta.y < Ec2.extent(1));
		assert(z-1+Ec2Delta.z < Ec2.extent(2));

		#pragma omp parallel for default(private) firstprivate(x,y,z)
		for(std::size_t i = 0; i < x; i++)
		{
			for(std::size_t j = 0; j < y; j++)
			{
				for(std::size_t k = 0; k < z; k++)
				{
					const auto Ec1i = i + Ec1Delta.x;
					const auto Ec1j = j + Ec1Delta.y;
					const auto Ec1k = k + Ec1Delta.z;

					const auto Ec2i = i + Ec2Delta.x;
					const auto Ec2j = j + Ec2Delta.y;
					const auto Ec2k = k + Ec2Delta.z;

					Hc[i,j,k] = Ch[i,j,k]*Hc[i,j,k] + Ce[i,j,k] *
						((Ec1[Ec1i,Ec1j,Ec1k]-Ec1[i,j,k]) - (Ec2[Ec2i,Ec2j,Ec2k]-Ec2[i,j,k]))
					;
				}
			}
		}
	}

	template<svec3 Hc1Delta, svec3 Hc2Delta>
	void updateEComponent(
		mdspan_3d_t Ec,
		cmdspan_3d_t Ce,
		cmdspan_3d_t Ch,
		cmdspan_3d_t Hc1,
		cmdspan_3d_t Hc2,
		svec3 start
	)
	{
		const std::size_t x = size.x-1;
		const std::size_t y = size.y-1;
		const std::size_t z = size.z-1;

		assert(start.x + Hc1Delta.x >= 0);
		assert(start.y + Hc1Delta.y >= 0);
		assert(start.z + Hc1Delta.z >= 0);

		assert(start.x + Hc2Delta.x >= 0);
		assert(start.y + Hc2Delta.y >= 0);
		assert(start.z + Hc2Delta.z >= 0);

		#pragma omp parallel for default(private) firstprivate(x,y,z)
		for(std::size_t i = start.x; i < x; i++)
		{
			for(std::size_t j = start.y; j < y; j++)
			{
				for(std::size_t k = start.z; k < z; k++)
				{
					const auto Hc1i = i + Hc1Delta.x;
					const auto Hc1j = j + Hc1Delta.y;
					const auto Hc1k = k + Hc1Delta.z;

					const auto Hc2i = i + Hc2Delta.x;
					const auto Hc2j = j + Hc2Delta.y;
					const auto Hc2k = k + Hc2Delta.z;

					Ec[i,j,k] = Ce[i,j,k]*Ec[i,j,k] + Ch[i,j,k] *
						((Hc1[i,j,k]-Hc1[Hc1i,Hc1j,Hc1k]) - (Hc2[i,j,k]-Hc2[Hc2i,Hc2j,Hc2k]))
					;
				}
			}
		}
	}

	void updateHx()
	{
		updateHComponent<-EzDimsDelta,-EyDimsDelta>(
			Hx(),
			Chxh(),
			Chxe(),
			Ey(),
			Ez()
		);
	}

	void updateHy()
	{
		updateHComponent<-ExDimsDelta,-EzDimsDelta>(
			Hy(),
			Chyh(),
			Chye(),
			Ez(),
			Ex()
		);
	}

	void updateHz()
	{
		updateHComponent<-EyDimsDelta,-ExDimsDelta>(
			Hz(),
			Chzh(),
			Chze(),
			Ex(),
			Ey()
		);
	}

	void updateEx()
	{
		updateEComponent<EyDimsDelta,EzDimsDelta>(
			Ex(),
			Cexe(),
			Cexh(),
			Hz(),
			Hy(),
			-HxDimsDelta
		);
	}

	void updateEy()
	{
		updateEComponent<EzDimsDelta,ExDimsDelta>(
			Ey(),
			Ceye(),
			Ceyh(),
			Hx(),
			Hz(),
			-HyDimsDelta
		);
	}

	void updateEz()
	{
		updateEComponent<ExDimsDelta,EyDimsDelta>(
			Ez(),
			Ceze(),
			Cezh(),
			Hy(),
			Hx(),
			-HzDimsDelta
		);
	}

	void updateH()
	{
		updateHx();
		updateHy();
		updateHz();
	}

	void updateE()
	{
		updateEx();
		updateEy();
		updateEz();
	}

	static T gauss(T time, T sigma, T x0 = 0)
	{
		T x = (time-x0)/sigma;

		if constexpr(std::is_arithmetic_v<T>)
			return std::exp(-(x*x));
		else
			return std::exp((float)-(x*x));
	}

	void gauss()
	{
		Ex()[gaussPosition.x, gaussPosition.y, gaussPosition.z] +=
			gauss(time, gaussSigma);
	}

	enum class Dim
	{
		X,
		Y,
		Z
	};

	template <Dim dim, typename T2, typename E, typename L, typename A>
	static auto slice(Kokkos::mdspan<T2, E, L, A> mat, std::size_t index = 0)
	{
		if constexpr(dim == Dim::X)
			return Kokkos::submdspan(mat, index, Kokkos::full_extent, Kokkos::full_extent);
		if constexpr(dim == Dim::Y)
			return Kokkos::submdspan(mat, Kokkos::full_extent, index, Kokkos::full_extent);
		if constexpr(dim == Dim::Z)
			return Kokkos::submdspan(mat, Kokkos::full_extent, Kokkos::full_extent, index);
	}

#ifndef NDEBUG
	template <typename T2, typename E, typename L, typename A>
	void debugPrint(std::string_view name, Kokkos::mdspan<T2, E, L, A> mat)
	{
		static_assert(mat.rank() == 2);

		std::println("{}: {}, size = {},{}", name, entt::type_id<typeof(mat)>().name(), mat.extent(0), mat.extent(1));

	}
#endif

	inline static T calculateSc(T Cr, T mu, T eps)
	{
		if constexpr(std::is_arithmetic_v<T>)
			return Cr/std::sqrt(mu*eps);
		else
			return Cr/std::sqrt((float)(mu*eps));
	}

	template <typename L1, typename L2, typename L3>
	void abcCommon(
		_mdspan_2d_t<L1> Ec,
		_cmdspan_2d_t<L1> Ecd,
		_cmdspan_2d_t<L2> mu,
		_cmdspan_2d_t<L3> eps,
		mdspan_2d_t ec
	)
	{
		assert(Ec.extents() == Ecd.extents());

		assert(Ec.extent(0) <= mu.extent(0));
		assert(Ec.extent(1) <= mu.extent(1));

		assert(Ec.extent(0) <= eps.extent(0));
		assert(Ec.extent(1) <= eps.extent(1));

		assert(Ec.extent(0) <= ec.extent(0));
		assert(Ec.extent(1) <= ec.extent(1));

#ifndef NDEBUG
		debugPrint("Ec", Ec);
		debugPrint("Ecd", Ecd);
		debugPrint("eps", eps);
		debugPrint("mu", mu);
		debugPrint("ec", ec);
		std::println();
#endif

		for(std::size_t i = 0; i < Ec.extent(0); i++)
		{
			for(std::size_t j = 0; j < Ec.extent(1); j++)
			{
				const T Sc      = calculateSc(Cr, mu[i,j], eps[i,j]);
				const T abcCoef = (Sc-1)/(Sc+1);

				Ec[i,j] = ec[i,j] + abcCoef*(Ecd[i,j]-Ec[i,j]);
				ec[i,j] = Ecd[i,j];
			}
		}
	}

	template <Dim dim>
	void abcSlicer(
		mdspan_3d_t Ec1,
		mdspan_3d_t Ec2,
		cmdspan_3d_t mu,
		cmdspan_3d_t eps,
		mdspan_2d_t e1,
		mdspan_2d_t e2,
		std::size_t sliceIndex,
		std::ptrdiff_t sliceDelta
	)
	{
		auto muSliced(slice<dim>(mu, sliceIndex));
		auto epsSliced(slice<dim>(eps, sliceIndex));

		abcCommon(slice<dim>(Ec1, sliceIndex), slice<dim>((cmdspan_3d_t)Ec1, sliceIndex+sliceDelta), muSliced, epsSliced, e1);
		abcCommon(slice<dim>(Ec2, sliceIndex), slice<dim>((cmdspan_3d_t)Ec2, sliceIndex+sliceDelta), muSliced, epsSliced, e2);
	}

	void abcX0()
	{
		abcSlicer<Dim::X>(Ey(), Ez(), muxR(), epsxR(), eyx0(), ezx0(), 1, 1);
	}

	void abcX1()
	{
		abcSlicer<Dim::X>(Ey(), Ez(), muxR(), epsxR(), eyx1(), ezx1(), size.x-1, -1);
	}

	void abcY0()
	{
		abcSlicer<Dim::Y>(Ex(), Ez(), muyR(), epsyR(), exy0(), ezy0(), 1, 1);
	}

	void abcY1()
	{
		abcSlicer<Dim::Y>(Ex(), Ez(), muyR(), epsyR(), exy1(), ezy1(), size.y-1, -1);
	}

	void abcZ0()
	{
		abcSlicer<Dim::Z>(Ex(), Ey(), muzR(), epszR(), exz0(), eyz0(), 1, 1);
	}

	void abcZ1()
	{
		abcSlicer<Dim::Z>(Ex(), Ey(), muzR(), epszR(), exz1(), eyz1(), size.z-1, -1);
	}

	void abcX()
	{
		abcX0();
		abcX1();
	}

	void abcY()
	{
		abcY0();
		abcY1();
	}

	void abcZ()
	{
		abcZ0();
		abcZ1();
	}

	void abc()
	{
		abcX();
		abcY();
		abcZ();
	}

};

}
