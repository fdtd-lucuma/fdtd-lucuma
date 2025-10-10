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

import lucuma.legacy_headers.mdspan;

import lucuma.utils;
import lucuma.services.basic;
import lucuma.services.vulkan;
import vulkan_hpp;

import :base;

import std;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

template <typename T>
class VulkanFdtdData
{
public:
	constexpr static auto HxDimsDelta = svec3(0, -1, -1);
	constexpr static auto HyDimsDelta = svec3(-1, 0, -1);
	constexpr static auto HzDimsDelta = svec3(-1, -1, 0);

	constexpr static auto ExDimsDelta = svec3(-1, 0, 0);
	constexpr static auto EyDimsDelta = svec3(0, -1, 0);
	constexpr static auto EzDimsDelta = svec3(0, 0, -1);

	template <typename extents, typename layout = Kokkos::layout_right>
	using mdspan_t = Kokkos::mdspan<T, extents, layout>;

	template <typename extents, typename layout = Kokkos::layout_right>
	using cmdspan_t = Kokkos::mdspan<const T, extents, layout>;

	using extents_2d_t = Kokkos::dextents<std::size_t, 2>;
	using extents_3d_t = Kokkos::dextents<std::size_t, 3>;

	template <typename layout = Kokkos::layout_right>
	using _mdspan_2d_t = mdspan_t<extents_2d_t, layout>;
	template <typename layout = Kokkos::layout_right>
	using _mdspan_3d_t = mdspan_t<extents_3d_t, layout>;

	template <typename layout = Kokkos::layout_right>
	using _cmdspan_2d_t = cmdspan_t<extents_2d_t, layout>;
	template <typename layout = Kokkos::layout_right>
	using _cmdspan_3d_t = cmdspan_t<extents_3d_t, layout>;

	using mdspan_2d_t = _mdspan_2d_t<>;
	using mdspan_3d_t = _mdspan_3d_t<>;

	using cmdspan_2d_t = _cmdspan_2d_t<>;
	using cmdspan_3d_t = _cmdspan_3d_t<>;

private:

	static inline auto toMdspan(vulkan::Buffer& buffer, svec3 paddedDims, svec3 dims)
	{
		return Kokkos::submdspan(mdspan_3d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y, paddedDims.z), dims.x, dims.y, dims.z);
	}

	static inline auto toMdspan(vulkan::Buffer& buffer, svec2 paddedDims, svec2 dims)
	{
		return Kokkos::submdspan(mdspan_2d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y), dims.x, dims.y);
	}

	static inline auto toMdspan(const vulkan::Buffer& buffer, svec3 paddedDims, svec3 dims)
	{
		return Kokkos::submdspan(cmdspan_3d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y, paddedDims.z), dims.x, dims.y, dims.z);
	}

	static inline auto toMdspan(const vulkan::Buffer& buffer, svec2 paddedDims, svec2 dims)
	{
		return Kokkos::submdspan(cmdspan_2d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y), dims.x, dims.y);
	}


public:

private:
};

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
requires (precision != Precision::f16) //TODO: Fix float16 vulkan device requirements
class Vulkan: public IBackend, public VulkanBase
{
public:
	using T = PrecisionTraits<precision>::type;

	Vulkan(Injector& injector):
		VulkanBase(injector)
	{ }


	virtual entt::entity init()
	{
		helloWorld();
		//TODO
		return entt::null;
	}

	virtual bool step(entt::entity id)
	{
		//TODO
		return false;
	}

	virtual void saveFiles(entt::entity id)
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

// Add one line for each new precision
//extern template class Vulkan<Precision::f16>;
extern template class Vulkan<Precision::f32>;
extern template class Vulkan<Precision::f64>;

}
