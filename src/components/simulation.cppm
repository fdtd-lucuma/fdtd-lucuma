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

export module lucuma.components:simulation;

import lucuma.utils;

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
