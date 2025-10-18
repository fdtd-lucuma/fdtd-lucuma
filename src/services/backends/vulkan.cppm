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

export module lucuma.services.backends:vulkan;

import lucuma.legacy_headers.entt;
import glm;

import lucuma.utils;
import lucuma.components;
import lucuma.services.basic;
import lucuma.services.vulkan;
import lucuma.services.backends.vulkan_components;
import vulkan_hpp;

import :base;

import std;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

export class VulkanBase
{
protected:
	VulkanBase(Injector& injector);

	vulkan::Allocator& vulkanAllocator;
	vulkan::Compute&   vulkanCompute;
	vulkan::All&       vulkanAll;
	basic::Settings&   settings;
	entt::registry&    registry;

	vulkan::SimpleCommandBuffer commandBuffer;

	void init();
	vulkan::CommandRecorder createCommandRecorder();

};

export template<Precision precision>
class Vulkan: public IBackend, public VulkanBase
{
public:
	using T = PrecisionTraits<precision>::type;
	using data_t = vulkan_components::FdtdData<T>;
	using create_info_t = typename data_t::create_info_t;

	Vulkan(Injector& injector):
		VulkanBase(injector)
	{ }


	virtual entt::entity init()
	{
		auto id = registry.create();

		create_info_t createInfo {
			.fdtdDataCreateInfo = {
				.size          = settings.size(),
				.gaussPosition = settings.size()/(std::uint64_t)2,

				//TODO: Get from settings
				.deltaT = (T)1,
				.imp0 = (T)377,
				.Cr = (T)(1.f/std::sqrt(3.f)),

				.maxTime = settings.time(),
				.gaussSigma = 10,
			},
			.compute = vulkanCompute,
			.allocator = vulkanAllocator,
		};

		data_t& data = registry.emplace<data_t>(id, createInfo);

		{
			auto recorder = createCommandRecorder();

			data.initCoefs(recorder);
		}

		return id;
	}

	virtual bool step(entt::entity id)
	{
		data_t& data = registry.get<data_t>(id);

		bool canContinue = data.step();

		if(canContinue)
		{
			std::println("Step #{}", data.getTime());

			{
				auto recorder = createCommandRecorder();

				data.updateH(recorder);
				data.updateE(recorder);
				data.gauss(recorder);
				data.abc(recorder);
			}

#ifndef NDEBUG
			for(auto&& [name, mat]: data.zippedFields())
				debugPrintSlice(name, mat, data.size);
#endif
		}

		return canContinue;
	}

	virtual void saveFiles(entt::entity id)
	{
		//TODO
	}

	virtual ~Vulkan() = default;

private:

};

// Add one line for each new precision
extern template class Vulkan<Precision::f16>;
extern template class Vulkan<Precision::f32>;
extern template class Vulkan<Precision::f64>;

}
