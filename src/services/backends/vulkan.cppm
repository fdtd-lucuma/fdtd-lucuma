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

#include <glm/gtx/component_wise.hpp> // Why is this not in the module?

export module lucuma.services.backends:vulkan;

import lucuma.legacy_headers.mdspan;
import lucuma.legacy_headers.entt;
import glm;

import lucuma.utils;
import lucuma.components;
import lucuma.services.basic;
import lucuma.services.vulkan;
import vulkan_hpp;
import vk_mem_alloc_hpp;

import :base;

import std;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

svec3 pad(svec3 size, svec3 workGroupSize);

template <typename T>
struct VulkanFdtdDataCreateInfo
{
	components::FdtdDataCreateInfo<T> fdtdDataCreateInfo;
	vulkan::Compute& compute;
	vulkan::Allocator& allocator;
};

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

	using create_info_t = VulkanFdtdDataCreateInfo<T>;
private:

	static inline auto toMdspan(vulkan::Buffer& buffer, svec3 paddedDims, svec3 dims)
	{
		return unpad(mdspan_3d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y, paddedDims.z), dims);
	}

	static inline auto toMdspan(vulkan::Buffer& buffer, svec2 paddedDims, svec2 dims)
	{
		return unpad(mdspan_2d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y), dims);
	}

	static inline auto toMdspan(const vulkan::Buffer& buffer, svec3 paddedDims, svec3 dims)
	{
		return unpad(cmdspan_3d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y, paddedDims.z), dims);
	}

	static inline auto toMdspan(const vulkan::Buffer& buffer, svec2 paddedDims, svec2 dims)
	{
		return unpad(cmdspan_2d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y), dims);
	}


	template <typename vec_t = svec3>
	static vulkan::Buffer makeBuffer(const create_info_t& createInfo, vec_t paddedDims, T defaultValue = (T)0, bool hostReadable = false)
	{
		vma::AllocationCreateFlags vmaFlags =
			vma::AllocationCreateFlagBits::eHostAccessSequentialWrite |
			vma::AllocationCreateFlagBits::eMapped;

		if(hostReadable)
			vmaFlags |= vma::AllocationCreateFlagBits::eHostAccessRandom;

		vulkan::Buffer result = createInfo.allocator.allocate(
			glm::compMul(paddedDims)*sizeof(T),
			vk::BufferUsageFlagBits::eStorageBuffer,
			vmaFlags
		);

		for(auto& x: result.getData<T>())
			x = defaultValue;

		return result;
	}

public:

	VulkanFdtdData(const create_info_t& createInfo):
		workGroupSize(createInfo.compute.getWorkgroupSize(createInfo.fdtdDataCreateInfo.size)),
		size(createInfo.fdtdDataCreateInfo.size),
		paddedSize(pad(size, workGroupSize)),
		gaussPosition(createInfo.fdtdDataCreateInfo.gaussPosition),
		deltaT(createInfo.fdtdDataCreateInfo.deltaT),
		imp0(createInfo.fdtdDataCreateInfo.imp0),
		Cr(createInfo.fdtdDataCreateInfo.Cr),
		maxTime(createInfo.fdtdDataCreateInfo.maxTime),
		gaussSigma(createInfo.fdtdDataCreateInfo.gaussSigma),
		HxDims(size + HxDimsDelta),
		HyDims(size + HyDimsDelta),
		HzDims(size + HzDimsDelta),
		ExDims(size + ExDimsDelta),
		EyDims(size + EyDimsDelta),
		EzDims(size + EzDimsDelta),
		eyxDims(EyDims.yz()),
		ezxDims(EzDims.yz()),
		exyDims(ExDims.xz()),
		ezyDims(EzDims.xz()),
		exzDims(ExDims.xy()),
		eyzDims(EyDims.xy())
	{
	}

	const svec3 workGroupSize;
	const svec3 size;
	const svec3 paddedSize;
	const svec3 gaussPosition;
	const T deltaT;
	const T imp0;
	const T Cr;

	const unsigned int maxTime;

private:
	unsigned int time = 0;
	T gaussSigma;

	// Magnetic field dimentions

	const svec3 HxDims;
	const svec3 HyDims;
	const svec3 HzDims;

	// Electric field dimentions

	const svec3 ExDims;
	const svec3 EyDims;
	const svec3 EzDims;

	// ABC dimentions

	const svec2 eyxDims;
	const svec2 ezxDims;

	const svec2 exyDims;
	const svec2 ezyDims;

	const svec2 exzDims;
	const svec2 eyzDims;
};

export class VulkanBase
{
protected:
	VulkanBase(Injector& injector);

	vulkan::Allocator& vulkanAllocator;
	vulkan::Compute&   vulkanCompute;
	vulkan::All&       vulkanAll;
	basic::Settings&   settings;
	entt::registry&    registry;

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
class Vulkan: public IBackend, public VulkanBase
{
public:
	using T = PrecisionTraits<precision>::type;
	using data_t = VulkanFdtdData<T>;
	using create_info_t = typename data_t::create_info_t;

	Vulkan(Injector& injector):
		VulkanBase(injector)
	{ }


	virtual entt::entity init()
	{
		helloWorld();
		auto id = registry.create();

		create_info_t createInfo {
			.fdtdDataCreateInfo = {
				.size          = settings.size(),
				.gaussPosition = settings.size()/(std::ptrdiff_t)2,

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

		data_t& d = registry.emplace<data_t>(id, createInfo);

		return id;
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
