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

#include <tracy/Tracy.hpp>

module lucuma.services.frontends;

import lucuma.utils;
import lucuma.services.backends;
import lucuma.legacy_headers.entt;
import lucuma.components;

import std;
import glm;

namespace lucuma::services::frontends
{

Headless::Headless([[maybe_unused]]Injector& injector):
	backend(injector.inject<backends::IBackend>()),
	settings(injector.inject<basic::Settings>()),
	registry(injector.inject<entt::registry>())
{ }

void Headless::compute()
{
	const auto id = registry.create();


	components::FdtdDataCreateInfo createInfo {
		.size          = settings.size(),
		.gaussPosition = settings.size()/(std::uint64_t)2,

		.deltaT = settings.deltaT(),
		.deltaX = settings.deltaX(),
		.deltaY = settings.deltaY(),
		.deltaZ = settings.deltaZ(),
		.imp0   = settings.imp0(),
		.Cr     = settings.Cr(),

		.maxTime = settings.time(),
		.gaussSigma = settings.gaussSigma(),

		// Magnetic fields

		.Hx0 = settings.Hx0(),
		.Hy0 = settings.Hy0(),
		.Hz0 = settings.Hz0(),

		.Chxh0 = settings.Chxh0(),
		.Chyh0 = settings.Chyh0(),
		.Chzh0 = settings.Chzh0(),

		.Chxe0 = settings.Chxe0(),
		.Chye0 = settings.Chye0(),
		.Chze0 = settings.Chze0(),

		.CMhx0 = settings.CMhx0(),
		.CMhy0 = settings.CMhy0(),
		.CMhz0 = settings.CMhz0(),

		.mux0 = settings.mux0(),
		.muy0 = settings.muy0(),
		.muz0 = settings.muz0(),

		.muxR0 = settings.muxR0(),
		.muyR0 = settings.muyR0(),
		.muzR0 = settings.muzR0(),

		// Electric fields

		.Ex0 = settings.Ex0(),
		.Ey0 = settings.Ey0(),
		.Ez0 = settings.Ez0(),

		.Cexe0 = settings.Cexe0(),
		.Ceye0 = settings.Ceye0(),
		.Ceze0 = settings.Ceze0(),

		.Cexh0 = settings.Cexh0(),
		.Ceyh0 = settings.Ceyh0(),
		.Cezh0 = settings.Cezh0(),

		.CEEx0 = settings.CEEx0(),
		.CEEy0 = settings.CEEy0(),
		.CEEz0 = settings.CEEz0(),

		.epsx0 = settings.epsx0(),
		.epsy0 = settings.epsy0(),
		.epsz0 = settings.epsz0(),

		.epsxR0 = settings.epsxR0(),
		.epsyR0 = settings.epsyR0(),
		.epszR0 = settings.epszR0(),

		// ABC's

		.eyx00 = settings.eyx00(),
		.ezx00 = settings.ezx00(),
		.eyx10 = settings.eyx10(),
		.ezx10 = settings.ezx10(),

		.exy00 = settings.exy00(),
		.ezy00 = settings.ezy00(),
		.exy10 = settings.exy10(),
		.ezy10 = settings.ezy10(),

		.exz00 = settings.exz00(),
		.eyz00 = settings.eyz00(),
		.exz10 = settings.exz10(),
		.eyz10 = settings.eyz10(),
	};

	backend.init(createInfo, id);

	while(backend.step(id))
	{
		if(settings.tracy())
			FrameMark;

		backend.saveFiles(id);
	}

	// TODO: RAII this
	registry.destroy(id);
}

}
