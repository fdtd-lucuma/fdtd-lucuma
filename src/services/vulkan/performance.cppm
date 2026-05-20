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

struct CounterData
{

#ifdef BUILD_FOR_TERMUX
	template<typename T, typename C = std::less<T>, typename A = std::allocator<T>>
	using set_t = std::set<T,C,A>;
#else
	template<typename T, typename C = std::less<T>, typename Co = std::vector<T>>
	using set_t = std::flat_set<T,C,Co>;
#endif

	vk::raii::QueryPool queryPool   = nullptr;
	std::uint32_t       queryPasses = {};

	set_t<std::uint32_t> enabledCounters = {};

	std::vector<vk::PerformanceCounterKHR>            counters     = {};
	std::vector<vk::PerformanceCounterDescriptionKHR> descriptions = {};

	std::vector<vk::PerformanceCounterResultKHR> results = {};

};

export class Performance
{
public:
	Performance(Injector& injector);

	void enableComputeCounters();

	void startRecording(vk::CommandBuffer buf);
	void submitPasses(vk::Queue queue, vk::CommandBuffer buf);

private:
	Core&            core;
	Device&          device;
	basic::Settings& settings;

	CounterData enableCounters(const QueueFamilyInfo& info);

	CounterData computeData  = {};

};


}
