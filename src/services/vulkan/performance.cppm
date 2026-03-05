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

export module lucuma.services.vulkan:performance;

import lucuma.utils;
import vulkan_hpp;
import lucuma.services.basic;

namespace lucuma::services::vulkan
{

using namespace lucuma::utils;
using namespace lucuma::services::basic;

struct QueueFamilyInfo;
class Core;
class Device;

export class Performance
{
public:
	Performance(Injector& injector);

	void enableComputeCounters();
	void enableGraphicsCounters();

private:
	Tracy&  tracy;
	Core&   core;
	Device& device;

	void enableCounters(const QueueFamilyInfo& info, vk::raii::QueryPool& queryPool, std::uint32_t& queryPasses);

	vk::raii::QueryPool computeQueryPool  = nullptr;
	vk::raii::QueryPool graphicsQueryPool = nullptr;

	std::uint32_t computeQueryPasses  = {};
	std::uint32_t graphicsQueryPasses = {};

};


}
