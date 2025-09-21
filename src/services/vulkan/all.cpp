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

module fdtd.services.vulkan;

import :utils;

namespace fdtd::services::vulkan
{

All::All(Injector& injector):
	core(injector.inject<Core>()),
	debug(injector.inject<Debug>()),
	device(injector.inject<Device>()),
	allocator(injector.inject<Allocator>()),
	shaderLoader(injector.inject<ShaderLoader>())
{

	list_vulkan_extensions();
	std::cout << device.getPhysicalDevice();

}

}
