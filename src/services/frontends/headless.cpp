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

	components::FdtdDataCreateInfo createInfo {
		.size          = settings.size(),
		.gaussPosition = settings.size()/(std::uint64_t)2,

		//TODO: Get from settings
		.deltaT = 1,
		.deltaX = 1,
		.deltaY = 1,
		.deltaZ = 1,
		.imp0 = 377,
		.Cr = 1.f/std::sqrt(3.f),

		.maxTime = settings.time(),
		.gaussSigma = 10,
	};

	auto id = backend.init(createInfo);

	while(backend.step(id))
	{
		backend.saveFiles(id);
	}

	// TODO: RAII this
	registry.destroy(id);
}

}
