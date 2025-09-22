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

import lucuma.utils;
import lucuma.services.basic;
import lucuma.services.vulkan;
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

	struct HelloWorldData
	{
		vulkan::ComputePipeline pipeline;

		vulkan::Buffer aBuffer;
		vulkan::Buffer bBuffer;
		vulkan::Buffer cBuffer;
	};

	HelloWorldData createHelloWorld(std::size_t bytes, std::string_view shaderPath);

};

template<Precision precision = Precision::f32>
constexpr std::string_view shaderPath()
{
	if constexpr(precision == Precision::f16)
		return "hello_world_half.spv";
	if constexpr(precision == Precision::f32)
		return "hello_world_float.spv";
	if constexpr(precision == Precision::f64)
		return "hello_world_double.spv";
}

export template<Precision precision>
class Vulkan: public Base, public VulkanBase
{
public:
	using T = PrecisionTraits<precision>::type;

	Vulkan(Injector& injector):
		VulkanBase(injector)
	{ }


	virtual void init()
	{
		helloWorld();
		//TODO
	}

	virtual bool step()
	{
		//TODO
		return false;
	}

	virtual void saveFiles()
	{
		//TODO
	}

	virtual ~Vulkan() = default;

private:

	void helloWorld()
	{
		auto generator = std::views::iota(0, 10);

		std::vector<T> a{std::from_range, generator};
		std::vector<T> b{std::from_range, generator | std::views::transform([](auto&& x){return x*x;})};

		auto pipeline = createHelloWorld(std::span(a).size_bytes(), shaderPath<precision>());

		pipeline.aBuffer.template setData<T>(a);
		pipeline.bBuffer.template setData<T>(b);

		auto& commandBuffer = pipeline.pipeline.getCommandBuffer();

		vk::CommandBufferBeginInfo beginInfo{};

		commandBuffer.begin(beginInfo);

		pipeline.pipeline.bind(commandBuffer);
		commandBuffer.dispatch(a.size(), 1, 1);

		commandBuffer.end();

		vulkanCompute.submit(commandBuffer);

		auto c = pipeline.cBuffer.template getData<T>().subspan(0, a.size());

		if constexpr(std::is_default_constructible_v<std::formatter<std::vector<T>>>)
			std::println("{} + {} = {}", a, b, c);
		else
		{
			auto toFloat = std::views::transform([](auto&& x){return (float)x;});

			std::println("{} + {} = {}",
				a | toFloat,
				b | toFloat,
				c | toFloat
			);
		}
	}


};


}
