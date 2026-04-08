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

#include <vulkan/vulkan.h>
#include <tracy/TracyVulkan.hpp>

export module lucuma.services.backends:vulkan;

import lucuma.legacy_headers.entt;
import glm;

import lucuma.utils;
import lucuma.events.vulkan;
import lucuma.components;
import lucuma.services.basic;
import lucuma.services.vulkan;
import lucuma.services.backends.vulkan_components;
import vulkan_hpp;

import :base;
import :saver;

import std;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

export class VulkanBase
{
protected:
	VulkanBase(Injector& injector);

	VulkanBase(VulkanBase const&) = delete;
	VulkanBase(VulkanBase&& other);

	VulkanBase& operator=(VulkanBase const&) = delete;

	vulkan::Allocator& vulkanAllocator;
	vulkan::Compute&   vulkanCompute;
	vulkan::Device&    vulkanDevice;
	vulkan::All&       vulkanAll;
	basic::Settings&   settings;
	basic::FileReader& fileReader;
	entt::registry&    registry;
	entt::dispatcher&  dispatcher;

	vulkan::SimpleCommandBuffer commandBuffer;

	void init();
	vulkan::CommandRecorder createCommandRecorder();

	static void computeComputeBarrier(vk::CommandBuffer commandBuffer);
	static void computeCpuBarrier(vk::CommandBuffer commandBuffer);

	void initCommon(const components::FdtdDataCreateInfo& createInfo, entt::entity id);

};

export template<Precision precision>
class Vulkan: public IBackend, public VulkanBase
{
public:
	using T = PrecisionTraits<precision>::type;
	using data_t = vulkan_components::FdtdData<T>;
	using saver_t = Saver<data_t>;
	using create_info_t = typename data_t::create_info_t;

	Vulkan(Injector& injector):
		VulkanBase(injector)
	{ }


	virtual void init(const components::FdtdDataCreateInfo& createInfo, entt::entity id)
	{
		initCommon(createInfo, id);

		create_info_t vulkanCreateInfo {
			.fdtdDataCreateInfo = createInfo,
			.compute            = vulkanCompute,
			.allocator          = vulkanAllocator,
			.device             = vulkanDevice,
		};

		SaverCreateInfo saverCreateInfo {
			.basePath = ".",
		};

		data_t& data = registry.emplace<data_t>(id, data_t::make(vulkanCreateInfo, fileReader));

		{
			auto recorder = createCommandRecorder();

			data.initBuffers(recorder);
			computeComputeBarrier(recorder);
			data.initCoefs(recorder);
			computeComputeBarrier(recorder);
		}

		// TODO: Find a way to dedup this
		if(settings.saveAs() != SaveAs::none)
		{
			saver_t& saver = registry.emplace<saver_t>(id, saverCreateInfo);
			saver.start(data);
		}

		if(settings.debug())
		{
			for(auto&& [name, mat]: data.chZippedFields())
				debugPrintSlice(name, mat, data.size);
			for(auto&& [name, mat]: data.ceZippedFields())
				debugPrintSlice(name, mat, data.size);
		}
	}

	virtual bool step(entt::entity id)
	{
		if(!registry.valid(id))
			return false;

		data_t& data = registry.get<data_t>(id);

		if(!data.step())
			return false;

		if(settings.debug())
			std::println("Step #{}", data.getTime());

		{
			auto recorder = createCommandRecorder();
			innerStep(id, data);
		}

		if(settings.debug())
		{
			for(auto&& [name, mat]: data.zippedFields())
				debugPrintSlice(name, mat, data.size);
		}

		return true;
	}

	virtual void saveFiles(entt::entity id)
	{
		// TODO: Sync Hx
		if(settings.saveAs() == SaveAs::none)
			return;

		auto [data, saver] = registry.get<data_t, saver_t>(id);

		saver.snapshot(data);
	}

	virtual ~Vulkan() = default;


private:

	void innerStep(entt::entity id, data_t& data)
	{
		auto* ctx = commandBuffer.getCtx();
		auto& cmdbuf = *commandBuffer.getCommandBuffer();

		TracyVkNamedZone(commandBuffer.getCtx(), __zone, *commandBuffer.getCommandBuffer(), "Compute", settings.tracy());

		{
			TracyVkNamedZone(ctx, __zone, cmdbuf, "H", settings.tracy());
			data.updateH(cmdbuf);
		}

		computeComputeBarrier(cmdbuf);

		{
			TracyVkNamedZone(ctx, __zone, cmdbuf, "E", settings.tracy());
			data.updateE(cmdbuf);
		}

		computeComputeBarrier(cmdbuf);

		{
			TracyVkNamedZone(ctx, __zone, cmdbuf, "gauss", settings.tracy());
			data.gauss(cmdbuf);
		}

		computeComputeBarrier(cmdbuf);

		{
			TracyVkNamedZone(ctx, __zone, cmdbuf, "abc", settings.tracy());
			data.abc(cmdbuf);
		}

		{
			TracyVkNamedZone(ctx, __zone, cmdbuf, "Post abc", settings.tracy());

			dispatcher.trigger(events::vulkan::PostFdtdAbc<precision>{
				.commandBuffer = commandBuffer,
				.handle        = entt::handle(registry, id),
				.ctx           = ctx,
			});
		}

		if(data.getTime() % 64 == 0 || data.getTime() == data.maxTime)
			commandBuffer.tracyCollect();

		if(settings.saveAs() != SaveAs::none)
			computeCpuBarrier(cmdbuf);

	}

};

// Add one line for each new precision
extern template class Vulkan<Precision::f16>;
extern template class Vulkan<Precision::f32>;
extern template class Vulkan<Precision::f64>;

}
