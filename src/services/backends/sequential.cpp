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

module lucuma.services.backends;

import lucuma.utils;
import std;
import glm;

namespace lucuma::services::backends
{

Sequential::Sequential([[maybe_unused]]Injector& injector):
	settings(injector.inject<basic::Settings>())
{ }

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

void Sequential::init()
{
	auto size = settings.size();

	// Magnetic field dimentions

	auto HxDims = size + HxDimsDelta;
	auto HyDims = size + HyDimsDelta;
	auto HzDims = size + HzDimsDelta;

	// Electric field dimentions

	auto ExDims = size + ExDimsDelta;
	auto EyDims = size + EyDimsDelta;
	auto EzDims = size + EzDimsDelta;

	// ABC dimentions

	auto eyxDims = EyDims.yz();
	auto ezxDims = EzDims.yz();

	auto exyDims = ExDims.xz();
	auto ezyDims = EzDims.xz();

	auto exzDims = ExDims.xy();
	auto eyzDims = EyDims.xy();

	// Magnetic fields

	std::vector<float> _Hx = initMat<float>(HxDims);
	std::vector<float> _Hy = initMat<float>(HyDims);
	std::vector<float> _Hz = initMat<float>(HzDims);

	std::vector<float> _Chxh = initMat<float>(HxDims);
	std::vector<float> _Chyh = initMat<float>(HyDims);
	std::vector<float> _Chzh = initMat<float>(HzDims);

	std::vector<float> _Chxe = initMat<float>(HxDims);
	std::vector<float> _Chye = initMat<float>(HyDims);
	std::vector<float> _Chze = initMat<float>(HzDims);

	std::vector<float> _CMhx = initMat<float>(HxDims);
	std::vector<float> _CMhy = initMat<float>(HyDims);
	std::vector<float> _CMhz = initMat<float>(HzDims);

	std::vector<float> _mux = initMat<float>(HxDims, 1.f);
	std::vector<float> _muy = initMat<float>(HyDims, 1.f);
	std::vector<float> _muz = initMat<float>(HzDims, 1.f);

	std::vector<float> _muxR = initMat<float>(HxDims, 1.f);
	std::vector<float> _muyR = initMat<float>(HyDims, 1.f);
	std::vector<float> _muzR = initMat<float>(HzDims, 1.f);

	// Electric fields

	std::vector<float> _Ex = initMat<float>(ExDims);
	std::vector<float> _Ey = initMat<float>(EyDims);
	std::vector<float> _Ez = initMat<float>(EzDims);

	std::vector<float> _Cexe = initMat<float>(ExDims);
	std::vector<float> _Ceye = initMat<float>(EyDims);
	std::vector<float> _Ceze = initMat<float>(EzDims);

	std::vector<float> _Cexh = initMat<float>(ExDims);
	std::vector<float> _Ceyh = initMat<float>(EyDims);
	std::vector<float> _Cezh = initMat<float>(EzDims);

	std::vector<float> _CEEx = initMat<float>(ExDims);
	std::vector<float> _CEEy = initMat<float>(EyDims);
	std::vector<float> _CEEz = initMat<float>(EzDims);

	std::vector<float> _epsx = initMat<float>(ExDims, 1.f);
	std::vector<float> _epsy = initMat<float>(EyDims, 1.f);
	std::vector<float> _epsz = initMat<float>(EzDims, 1.f);

	std::vector<float> _epsxR = initMat<float>(ExDims, 1.f);
	std::vector<float> _epsyR = initMat<float>(EyDims, 1.f);
	std::vector<float> _epszR = initMat<float>(EzDims, 1.f);

	// ABC's

	std::vector<float> _eyx0 = initMat<float>(eyxDims);
	std::vector<float> _ezx0 = initMat<float>(ezxDims);
	std::vector<float> _eyx1 = initMat<float>(eyxDims);
	std::vector<float> _ezx1 = initMat<float>(ezxDims);

	std::vector<float> _exy0 = initMat<float>(exyDims);
	std::vector<float> _ezy0 = initMat<float>(ezyDims);
	std::vector<float> _exy1 = initMat<float>(exyDims);
	std::vector<float> _ezy1 = initMat<float>(ezyDims);

	std::vector<float> _exz0 = initMat<float>(exzDims);
	std::vector<float> _eyz0 = initMat<float>(eyzDims);
	std::vector<float> _exz1 = initMat<float>(exzDims);
	std::vector<float> _eyz1 = initMat<float>(eyzDims);

	//TODO
}

bool Sequential::step()
{
	//TODO
	return false;
}

void Sequential::saveFiles()
{
	//TODO
}


}
