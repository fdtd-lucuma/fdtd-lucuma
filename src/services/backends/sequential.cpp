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
