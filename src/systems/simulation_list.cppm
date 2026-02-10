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

export module lucuma.systems:simulation_list;

import lucuma.services.basic;
import lucuma.services.backends;
import lucuma.utils;

import :base;

import std;

namespace lucuma::systems
{

using namespace lucuma::utils;

using namespace services::basic;
using namespace services::backends;

struct FdtdSimulationInfo
{
	// Basic
	float basicSize[3];
	float basicGaussPosition[3];
	float basicTime; // In ns
	float basicDeltaSize[3]; // In mm
	float epsilon;

	// Advanced
	unsigned int size[3];
	unsigned int gaussPosition[3];
	float        deltaT;
	float        imp0;
	float        Cr;
	unsigned int maxTime;
	float        gaussSigma;

	Backend   backend;
	Precision precision;

	// Magnetic fields
	std::filesystem::path Hx0;
	std::filesystem::path Hy0;
	std::filesystem::path Hz0;

	std::filesystem::path Chxh0;
	std::filesystem::path Chyh0;
	std::filesystem::path Chzh0;

	std::filesystem::path Chxe0;
	std::filesystem::path Chye0;
	std::filesystem::path Chze0;

	std::filesystem::path CMhx0;
	std::filesystem::path CMhy0;
	std::filesystem::path CMhz0;

	std::filesystem::path mux0;
	std::filesystem::path muy0;
	std::filesystem::path muz0;

	std::filesystem::path muxR0;
	std::filesystem::path muyR0;
	std::filesystem::path muzR0;

	// Electric fields

	std::filesystem::path Ex0;
	std::filesystem::path Ey0;
	std::filesystem::path Ez0;

	std::filesystem::path Cexe0;
	std::filesystem::path Ceye0;
	std::filesystem::path Ceze0;

	std::filesystem::path Cexh0;
	std::filesystem::path Ceyh0;
	std::filesystem::path Cezh0;

	std::filesystem::path CEEx0;
	std::filesystem::path CEEy0;
	std::filesystem::path CEEz0;

	std::filesystem::path epsx0;
	std::filesystem::path epsy0;
	std::filesystem::path epsz0;

	std::filesystem::path epsxR0;
	std::filesystem::path epsyR0;
	std::filesystem::path epszR0;

	// ABC's

	std::filesystem::path eyx00;
	std::filesystem::path ezx00;
	std::filesystem::path eyx10;
	std::filesystem::path ezx10;

	std::filesystem::path exy00;
	std::filesystem::path ezy00;
	std::filesystem::path exy10;
	std::filesystem::path ezy10;

	std::filesystem::path exz00;
	std::filesystem::path eyz00;
	std::filesystem::path exz10;
	std::filesystem::path eyz10;
};

export
class SimulationList: public Base<SimulationList>
{
public:
	SimulationList(Systems& _systems);

	void update(const events::Update& event);

private:
	Settings&       settings;
	Instantiator&   instantiator;
	entt::registry& registry;

	FdtdSimulationInfo newSimulationInfo = {};

	void init();
	void newSimulationPopup();
	void newSimulationInputs();
	void newSimulationBackends();
	void resetNewSimulationInfo();
	void startSimulation();
	void clampSizes();
	void simulationTable();

	void rowActions(entt::entity id);
};

}
