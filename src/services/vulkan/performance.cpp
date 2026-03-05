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

#include <vulkan/vulkan_core.h>

module lucuma.services.vulkan;

namespace lucuma::services::vulkan
{

Performance::Performance([[maybe_unused]] Injector& injector):
	tracy(injector.inject<Tracy>()),
	core(injector.inject<Core>()),
	device(injector.inject<Device>())
{
}

void Performance::enableCounters(const QueueFamilyInfo& info, vk::raii::QueryPool& queryPool)
{
	auto [counters, descriptions] = core.getPhysicalDevice().enumerateQueueFamilyPerformanceQueryCountersKHR(info.index);

	for(size_t i = 0; i < counters.size(); i++)
	{
		std::println("{}:  {}, {}, {}, {}, {}, {}, {}", i,
			counters[i].unit,
			counters[i].scope,
			counters[i].storage,
			to_string(descriptions[i].flags),
			descriptions[i].name.begin(),
			descriptions[i].category.begin(),
			descriptions[i].description.begin()
		);
	}
}

void Performance::enableComputeCounters()
{
	auto queueInfo = device.getComputeInfo();

	if(queueInfo.has_value())
		enableCounters(queueInfo.value(), computeQueryPool);
}

void Performance::enableGraphicsCounters()
{
	auto queueInfo = device.getGraphicsInfo();

	if(queueInfo.has_value())
		enableCounters(queueInfo.value(), graphicsQueryPool);
}


}
