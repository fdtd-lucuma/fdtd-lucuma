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

#include <cassert>

module lucuma.services.vulkan;

namespace lucuma::services::vulkan
{

Performance::Performance([[maybe_unused]] Injector& injector):
	core(injector.inject<Core>()),
	device(injector.inject<Device>()),
	settings(injector.inject<basic::Settings>())
{
}

CounterData Performance::enableCounters(const QueueFamilyInfo& info)
{
	auto [counters, descriptions] = core.getPhysicalDevice().enumerateQueueFamilyPerformanceQueryCountersKHR(info.index);

	if(settings.debug())
	{
		for(std::size_t i = 0; i < counters.size(); i++)
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


	auto enabledSet =
		std::views::iota((std::uint32_t)0, (std::uint32_t)descriptions.size()) |
		std::views::filter([&](auto&& i) {return !(descriptions[i].flags & vk::PerformanceCounterDescriptionFlagBitsKHR::ePerformanceImpacting);}) |
		std::ranges::to<std::flat_set>()
	;

	auto enabled{std::move(enabledSet).extract()};

	auto enabledSize = enabled.size();

	auto chain = vk::StructureChain {
		vk::QueryPoolCreateInfo {
			.queryType  = vk::QueryType::ePerformanceQueryKHR,
			.queryCount = 1,
		},
		vk::QueryPoolPerformanceCreateInfoKHR {
			.queueFamilyIndex = info.index,
		}
	};

#ifdef __APPLE__
	auto& queryPoolCreateInfo   = chain.get<vk::QueryPoolCreateInfo>();
	auto& performanceCreateInfo = chain.get<vk::QueryPoolPerformanceCreateInfoKHR>();
#else
	auto& [queryPoolCreateInfo, performanceCreateInfo] = chain;
#endif

	performanceCreateInfo.setCounterIndices(enabled);

	auto queryPool   = device.getDevice().createQueryPool(queryPoolCreateInfo);
	auto queryPasses = core.getPhysicalDevice().getQueueFamilyPerformanceQueryPassesKHR(performanceCreateInfo);

	return {
		.queryPool       = std::move(queryPool),
		.queryPasses     = queryPasses,
		.enabledCounters = std::move(enabled),
		.counters        = std::move(counters),
		.descriptions    = std::move(descriptions),
		.results         = std::vector<vk::PerformanceCounterResultKHR>(enabledSize),
	};

}

void Performance::enableComputeCounters()
{
	auto queueInfo = device.getComputeInfo();

	if(queueInfo.has_value())
		computeData = enableCounters(queueInfo.value());
}

void Performance::startRecording(vk::CommandBuffer buf)
{
	buf.reset();

	vk::CommandBufferBeginInfo beginInfo {
	};


	vk::AcquireProfilingLockInfoKHR lockInfo {
		.timeout = std::numeric_limits<std::uint64_t>::max(),
	};

	device.getDevice().acquireProfilingLockKHR(lockInfo);

	buf.begin(beginInfo);
	computeData.queryPool.reset(0, 1);
	//buf.resetQueryPool(computeData.queryPool, 0, 1);
	buf.beginQuery(computeData.queryPool, 0, {});

}

void Performance::submitPasses(vk::Queue queue, vk::CommandBuffer buf)
{
	buf.pipelineBarrier(vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, {}, {}, {}, {});
	buf.endQuery(computeData.queryPool, 0);
	buf.end();

	for(std::uint32_t pass = 0; pass < computeData.queryPasses; pass++)
	{
		vk::StructureChain chain {
			vk::SubmitInfo {
			},
			vk::PerformanceQuerySubmitInfoKHR {
				.counterPassIndex = pass,
			},
		};

#ifdef __APPLE__
		auto& submitInfo       = chain.get<vk::SubmitInfo>();
		auto& performanceQuery = chain.get<vk::PerformanceQuerySubmitInfoKHR>();
#else
		auto& [submitInfo, performanceQuery] = chain;
#endif

		submitInfo.setCommandBuffers(buf);

		queue.submit(submitInfo);
		queue.waitIdle(); //TODO: Use proper synchronization
	}

	device.getDevice().releaseProfilingLockKHR();
	buf.reset();

	auto size = sizeof(vk::PerformanceCounterResultKHR)*computeData.enabledCounters.size();
	computeData.results = computeData.queryPool.getResults<vk::PerformanceCounterResultKHR>(0, 1, size, size).value;

	assert(computeData.results.size() == computeData.enabledCounters.size());

	for(auto i: computeData.enabledCounters)
	{
		auto& counter     = computeData.counters[i];
		auto& description = computeData.descriptions[i];
		auto& result      = computeData.results[i];

		switch(counter.storage)
		{
			using enum vk::PerformanceCounterStorageKHR;

			case eInt32:
				std::println("{}: {}", description.name.begin(), result.int32);
				break;

			case eInt64:
				std::println("{}: {}", description.name.begin(), result.int64);
				break;

			case eUint32:
				std::println("{}: {}", description.name.begin(), result.uint32);
				break;

			case eUint64:
				std::println("{}: {}", description.name.begin(), result.uint64);
				break;

			case eFloat32:
				std::println("{}: {}", description.name.begin(), result.float32);
				break;

			case eFloat64:
				std::println("{}: {}", description.name.begin(), result.float64);
				break;
		};
	}

}

}
