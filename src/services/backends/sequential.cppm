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

export module lucuma.services.backends:sequential;

import lucuma.utils;
import lucuma.services.basic;

import :base;

import std;
import glm;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

template <typename T>
requires std::is_arithmetic_v<T>
std::vector<T> initMat(svec3 dims, T defaultValue = 0)
{
	return std::vector<T>(dims.x*dims.y*dims.z, defaultValue);
}

template <typename T>
requires std::is_arithmetic_v<T>
std::vector<T> initMat(svec2 dims, T defaultValue = 0)
{
	return std::vector<T>(dims.x*dims.y, defaultValue);
}

export class Sequential: public Base
{
public:
	Sequential(Injector& injector);

	virtual void init();
	virtual bool step();
	virtual void saveFiles();

	virtual ~Sequential() = default;

	constexpr static auto HxDimsDelta = svec3(0, -1, -1);
	constexpr static auto HyDimsDelta = svec3(-1, 0, -1);
	constexpr static auto HzDimsDelta = svec3(-1, -1, 0);

	constexpr static auto ExDimsDelta = svec3(-1, 0, 0);
	constexpr static auto EyDimsDelta = svec3(0, -1, 0);
	constexpr static auto EzDimsDelta = svec3(0, 0, -1);

private:
	basic::Settings& settings;

	template <typename T>
	requires std::is_arithmetic_v<T>
	struct FdtdData
	{
	public:
		FdtdData(svec3 _size):
			size(_size),
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
			_muxR(initMat<T>(HxDims, 1)),
			_muyR(initMat<T>(HyDims, 1)),
			_muzR(initMat<T>(HzDims, 1)),
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
			_epsxR(initMat<T>(ExDims, 1)),
			_epsyR(initMat<T>(EyDims, 1)),
			_epszR(initMat<T>(EzDims, 1)),
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

	private:
		svec3 size;

		// Magnetic field dimentions

		svec3 HxDims;
		svec3 HyDims;
		svec3 HzDims;

		// Electric field dimentions

		svec3 ExDims;
		svec3 EyDims;
		svec3 EzDims;

		// ABC dimentions

		svec2 eyxDims;
		svec2 ezxDims;

		svec2 exyDims;
		svec2 ezyDims;

		svec2 exzDims;
		svec2 eyzDims;

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
	};

};

}
