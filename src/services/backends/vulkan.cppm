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
import :utils;

import std;

namespace lucuma::services::backends
{

using namespace lucuma::utils;

template <typename T>
constexpr std::string_view shaderSuffix()
{
	if constexpr(std::is_same_v<T, PrecisionTraits<Precision::f16>::type>)
		return "_half.spv";
	if constexpr(std::is_same_v<T, PrecisionTraits<Precision::f32>::type>)
		return "_float.spv";
	if constexpr(std::is_same_v<T, PrecisionTraits<Precision::f64>::type>)
		return "_double.spv";
}


template <typename T>
std::filesystem::path shaderName(std::string_view name)
{
	return std::format("{}{}", name, shaderSuffix<T>());
}


svec3 pad(svec3 size, svec3 workGroupSize);
svec2 pad(svec2 size, svec2 workGroupSize);

template <typename T>
struct InitCoefPipelineCreateInfo
{
	svec3 paddedDims;
	svec3 dims;
	T CrImp;

	vulkan::Buffer& Ch;
	vulkan::Buffer& Ce;
	vulkan::Buffer& CM;
	vulkan::Buffer& mu;

	std::filesystem::path shaderPath;
	std::string_view      entrypoint = "main";
	svec3                 workGroupSize;

	vulkan::Compute& compute;
};

template <typename T>
class InitCoefPipeline
{
private:
	struct alignas(32)
	{
		alignas(sizeof(svec4)) svec3 paddedDims;
		alignas(sizeof(svec4)) svec3 dims;
		T CrImp;

		constexpr auto getPushConstantsLayout()
		{
			using this_t = typeof(pushConstants);

			return vulkan::Compute::makePushConstantsLayout<this_t>(
				&this_t::paddedDims,
				&this_t::dims,
				&this_t::CrImp
			);
		}

	} pushConstants;

	svec3 groupCount;

	vulkan::Buffer& Ch;
	vulkan::Buffer& Ce;
	vulkan::Buffer& CM;
	vulkan::Buffer& mu;

	vulkan::ComputePipeline pipeline;

public:
	InitCoefPipeline(const InitCoefPipelineCreateInfo<T>& createInfo):
		pushConstants({
			.paddedDims = createInfo.paddedDims,
			.dims = createInfo.dims,
			.CrImp = createInfo.CrImp,
		}),
		groupCount(createInfo.paddedDims/createInfo.workGroupSize),
		Ch(createInfo.Ch),
		Ce(createInfo.Ce),
		CM(createInfo.CM),
		mu(createInfo.mu),
		pipeline(createInfo.compute.createPipeline({
			.shaderPath = createInfo.shaderPath,
			.setLayouts = {
				{
					.bindings = simpleStorageBuffersLayout<4>(),
					.buffers = {
						createInfo.Ch,
						createInfo.Ce,
						createInfo.CM,
						createInfo.mu,
					}
				}
			},
			.workGroupSize = createInfo.workGroupSize,
			.pushConstants = pushConstants.getPushConstantsLayout(),
		}))
	{ }

	void dispatch(vk::CommandBuffer commandBuffer)
	{
		pipeline.bind(commandBuffer);
		pipeline.pushConstants(commandBuffer, pushConstants);
		commandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
	}

};

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

	// Padded vulkan buffers

	using vmdspan_2d_t = _mdspan_2d_t<>;
	using vmdspan_3d_t = _mdspan_3d_t<>;

	using vcmdspan_2d_t = _cmdspan_2d_t<>;
	using vcmdspan_3d_t = _cmdspan_3d_t<>;


	// Unpadded vulkan buffers
	using mdspan_2d_t = _mdspan_2d_t<Kokkos::layout_stride>;
	using mdspan_3d_t = _mdspan_3d_t<Kokkos::layout_stride>;

	using cmdspan_2d_t = _cmdspan_2d_t<Kokkos::layout_stride>;
	using cmdspan_3d_t = _cmdspan_3d_t<Kokkos::layout_stride>;

	using create_info_t = VulkanFdtdDataCreateInfo<T>;

	using MatrixData = vulkan::Buffer;
private:

	static inline mdspan_3d_t toMdspan(vulkan::Buffer& buffer, svec3 paddedDims, svec3 dims)
	{
		return unpad(vmdspan_3d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y, paddedDims.z), dims);
	}

	static inline mdspan_2d_t toMdspan(vulkan::Buffer& buffer, svec2 paddedDims, svec2 dims)
	{
		return unpad(vmdspan_2d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y), dims);
	}

	static inline cmdspan_3d_t toMdspan(const vulkan::Buffer& buffer, svec3 paddedDims, svec3 dims)
	{
		return unpad(vcmdspan_3d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y, paddedDims.z), dims);
	}

	static inline cmdspan_2d_t toMdspan(const vulkan::Buffer& buffer, svec2 paddedDims, svec2 dims)
	{
		return unpad(vcmdspan_2d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y), dims);
	}


	template <typename vec_t = svec3>
	static vulkan::Buffer makeBuffer(const create_info_t& createInfo, vec_t paddedDims, T defaultValue = (T)0, bool hostReadable = false)
	{
		vma::AllocationCreateFlags vmaFlags = vma::AllocationCreateFlagBits::eMapped;

		if(hostReadable)
			vmaFlags |= vma::AllocationCreateFlagBits::eHostAccessRandom;
		else
			vmaFlags |= vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;

		vulkan::Buffer result = createInfo.allocator.allocate(
			glm::compMul(paddedDims)*sizeof(T),
			vk::BufferUsageFlagBits::eStorageBuffer,
			vmaFlags
		);

		// TODO: Init from a shader
		for(auto& x: result.getData<T>())
			x = defaultValue;

		return result;
	}

	template <typename vec_t = svec3>
	static vulkan::Buffer makeRWBuffer(const create_info_t& createInfo, vec_t paddedDims, T defaultValue = (T)0)
	{
		return makeBuffer(createInfo, paddedDims, defaultValue, true);
	}

	svec3 padDims(svec3 vec)
	{
		return pad(vec, workGroupSize);
	}

	svec2 padYZDims(svec2 dims)
	{
		return pad(dims, workGroupSize.yz());
	}

	svec2 padXZDims(svec2 dims)
	{
		return pad(dims, workGroupSize.xz());
	}

	svec2 padXYDims(svec2 dims)
	{
		return pad(dims, workGroupSize.xy());
	}

public:

	VulkanFdtdData(const create_info_t& createInfo):
		workGroupSize(createInfo.compute.getWorkgroupSize(createInfo.fdtdDataCreateInfo.size)),
		size(createInfo.fdtdDataCreateInfo.size),
		paddedSize(padDims(size)),
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
		eyzDims(EyDims.xy()),
		paddedHxDims(padDims(HxDims)),
		paddedHyDims(padDims(HyDims)),
		paddedHzDims(padDims(HzDims)),
		paddedExDims(padDims(ExDims)),
		paddedEyDims(padDims(EyDims)),
		paddedEzDims(padDims(EzDims)),
		paddedEyxDims(padYZDims(eyxDims)),
		paddedEzxDims(padYZDims(ezxDims)),
		paddedExyDims(padXZDims(exyDims)),
		paddedEzyDims(padXZDims(ezyDims)),
		paddedExzDims(padXYDims(exzDims)),
		paddedEyzDims(padXYDims(eyzDims)),
		_Hx(makeRWBuffer(createInfo, paddedHxDims)),
		_Hy(makeRWBuffer(createInfo, paddedHyDims)),
		_Hz(makeRWBuffer(createInfo, paddedHzDims)),
		_Chxh(makeBuffer(createInfo, paddedHxDims)),
		_Chyh(makeBuffer(createInfo, paddedHyDims)),
		_Chzh(makeBuffer(createInfo, paddedHzDims)),
		_Chxe(makeBuffer(createInfo, paddedHxDims)),
		_Chye(makeBuffer(createInfo, paddedHyDims)),
		_Chze(makeBuffer(createInfo, paddedHzDims)),
		_CMhx(makeBuffer(createInfo, paddedHxDims)),
		_CMhy(makeBuffer(createInfo, paddedHyDims)),
		_CMhz(makeBuffer(createInfo, paddedHzDims)),
		_mux(makeBuffer(createInfo, paddedHxDims, 1)),
		_muy(makeBuffer(createInfo, paddedHyDims, 1)),
		_muz(makeBuffer(createInfo, paddedHzDims, 1)),
		_muxR(makeBuffer(createInfo, paddedSize, 1)),
		_muyR(makeBuffer(createInfo, paddedSize, 1)),
		_muzR(makeBuffer(createInfo, paddedSize, 1)),
		_Ex(makeRWBuffer(createInfo, paddedExDims)),
		_Ey(makeRWBuffer(createInfo, paddedEyDims)),
		_Ez(makeRWBuffer(createInfo, paddedEzDims)),
		_Cexe(makeBuffer(createInfo, paddedExDims)),
		_Ceye(makeBuffer(createInfo, paddedEyDims)),
		_Ceze(makeBuffer(createInfo, paddedEzDims)),
		_Cexh(makeBuffer(createInfo, paddedExDims)),
		_Ceyh(makeBuffer(createInfo, paddedEyDims)),
		_Cezh(makeBuffer(createInfo, paddedEzDims)),
		_CEEx(makeBuffer(createInfo, paddedExDims)),
		_CEEy(makeBuffer(createInfo, paddedEyDims)),
		_CEEz(makeBuffer(createInfo, paddedEzDims)),
		_epsx(makeBuffer(createInfo, paddedExDims, 1)),
		_epsy(makeBuffer(createInfo, paddedEyDims, 1)),
		_epsz(makeBuffer(createInfo, paddedEzDims, 1)),
		_epsxR(makeBuffer(createInfo, paddedSize, 1)),
		_epsyR(makeBuffer(createInfo, paddedSize, 1)),
		_epszR(makeBuffer(createInfo, paddedSize, 1)),
		_eyx0(makeBuffer(createInfo, paddedEyxDims)),
		_ezx0(makeBuffer(createInfo, paddedEzxDims)),
		_eyx1(makeBuffer(createInfo, paddedEyxDims)),
		_ezx1(makeBuffer(createInfo, paddedEzxDims)),
		_exy0(makeBuffer(createInfo, paddedExyDims)),
		_ezy0(makeBuffer(createInfo, paddedEzyDims)),
		_exy1(makeBuffer(createInfo, paddedExyDims)),
		_ezy1(makeBuffer(createInfo, paddedEzyDims)),
		_exz0(makeBuffer(createInfo, paddedExzDims)),
		_eyz0(makeBuffer(createInfo, paddedEyzDims)),
		_exz1(makeBuffer(createInfo, paddedExzDims)),
		_eyz1(makeBuffer(createInfo, paddedEyzDims)),
		initCoefHxPipeline(InitCoefPipelineCreateInfo<T>{
			.paddedDims = paddedHxDims,
			.dims = HxDims,
			.CrImp = Cr/imp0,
			.Ch = _Chxh,
			.Ce = _Chxe,
			.CM = _CMhx,
			.mu = _mux,
			.shaderPath = shaderName<T>("init_coefs"),
			.workGroupSize = workGroupSize,
			.compute = createInfo.compute,
		})//,
		//initCoefHyPipeline({
		//	.paddedDims = paddedHyDims,
		//	.dims = HyDims,
		//	.CrImp = Cr/imp0,
		//	.workGroupSize = workGroupSize,
		//	.Ch = _Chyh,
		//	.Ce = _Chye,
		//	.CM = _CMhy,
		//	.mu = _muy,
		//	.shaderPath = "init_coefs",
		//	.compute = createInfo.compute,
		//}),
		//initCoefHzPipeline({
		//	.paddedDims = paddedHzDims,
		//	.dims = HzDims,
		//	.CrImp = Cr/imp0,
		//	.workGroupSize = workGroupSize,
		//	.Ch = _Chzh,
		//	.Ce = _Chze,
		//	.CM = _CMhz,
		//	.mu = _muz,
		//	.shaderPath = "init_coefs",
		//	.compute = createInfo.compute,
		//}),
		//initCoefExPipeline({
		//	.paddedDims = paddedExDims,
		//	.dims = ExDims,
		//	.CrImp = Cr*imp0,
		//	.workGroupSize = workGroupSize,
		//	.Ch = _Cexe,
		//	.Ce = _Cexh,
		//	.CM = _CEEx,
		//	.mu = _epsx,
		//	.shaderPath = "init_coefs",
		//	.compute = createInfo.compute,
		//}),
		//initCoefEyPipeline({
		//	.paddedDims = paddedEyDims,
		//	.dims = EyDims,
		//	.CrImp = Cr*imp0,
		//	.workGroupSize = workGroupSize,
		//	.Ch = _Ceye,
		//	.Ce = _Ceyh,
		//	.CM = _CEEy,
		//	.mu = _epsy,
		//	.shaderPath = "init_coefs",
		//	.compute = createInfo.compute,
		//}),
		//initCoefEzPipeline({
		//	.paddedDims = paddedEzDims,
		//	.dims = EzDims,
		//	.CrImp = Cr*imp0,
		//	.workGroupSize = workGroupSize,
		//	.Ch = _Ceze,
		//	.Ce = _Cezh,
		//	.CM = _CEEz,
		//	.mu = _epsz,
		//	.shaderPath = "init_coefs",
		//	.compute = createInfo.compute,
		//})
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

	// Padded Magnetic field dimentions

	const svec3 paddedHxDims;
	const svec3 paddedHyDims;
	const svec3 paddedHzDims;

	// Padded Electric field dimentions

	const svec3 paddedExDims;
	const svec3 paddedEyDims;
	const svec3 paddedEzDims;

	// Padded ABC dimentions

	const svec2 paddedEyxDims;
	const svec2 paddedEzxDims;

	const svec2 paddedExyDims;
	const svec2 paddedEzyDims;

	const svec2 paddedExzDims;
	const svec2 paddedEyzDims;

	MatrixData _Hx;
	MatrixData _Hy;
	MatrixData _Hz;

	MatrixData _Chxh;
	MatrixData _Chyh;
	MatrixData _Chzh;

	MatrixData _Chxe;
	MatrixData _Chye;
	MatrixData _Chze;

	MatrixData _CMhx;
	MatrixData _CMhy;
	MatrixData _CMhz;

	MatrixData _mux;
	MatrixData _muy;
	MatrixData _muz;

	MatrixData _muxR;
	MatrixData _muyR;
	MatrixData _muzR;

	// Electric fields

	MatrixData _Ex;
	MatrixData _Ey;
	MatrixData _Ez;

	MatrixData _Cexe;
	MatrixData _Ceye;
	MatrixData _Ceze;

	MatrixData _Cexh;
	MatrixData _Ceyh;
	MatrixData _Cezh;

	MatrixData _CEEx;
	MatrixData _CEEy;
	MatrixData _CEEz;

	MatrixData _epsx;
	MatrixData _epsy;
	MatrixData _epsz;

	MatrixData _epsxR;
	MatrixData _epsyR;
	MatrixData _epszR;

	// ABC's

	MatrixData _eyx0;
	MatrixData _ezx0;
	MatrixData _eyx1;
	MatrixData _ezx1;

	MatrixData _exy0;
	MatrixData _ezy0;
	MatrixData _exy1;
	MatrixData _ezy1;

	MatrixData _exz0;
	MatrixData _eyz0;
	MatrixData _exz1;
	MatrixData _eyz1;

	InitCoefPipeline<T> initCoefHxPipeline;
	//InitCoefPipeline<T> initCoefHyPipeline;
	//InitCoefPipeline<T> initCoefHzPipeline;

	//InitCoefPipeline<T> initCoefExPipeline;
	//InitCoefPipeline<T> initCoefEyPipeline;
	//InitCoefPipeline<T> initCoefEzPipeline;

public:

	cmdspan_3d_t Hx() const { return toMdspan(_Hx, paddedHxDims, HxDims); }
	cmdspan_3d_t Hy() const { return toMdspan(_Hy, paddedHyDims, HyDims); }
	cmdspan_3d_t Hz() const { return toMdspan(_Hz, paddedHzDims, HzDims); }
	cmdspan_3d_t Ex() const { return toMdspan(_Ex, paddedExDims, ExDims); }
	cmdspan_3d_t Ey() const { return toMdspan(_Ey, paddedEyDims, EyDims); }
	cmdspan_3d_t Ez() const { return toMdspan(_Ez, paddedEzDims, EzDims); }

	/// Returns true and increments the counter by +1 if it can still continue.
	bool step() {
		if(time >= maxTime)
			return false;

		time++;
		return true;
	}

	unsigned int getTime() const
	{
		return time;
	}

	std::generator<std::tuple<const char*, cmdspan_3d_t>> zippedFields() const {
		static constexpr std::array names {
			"Hx",
			"Hy",
			"Hz",
			"Ex",
			"Ey",
			"Ez",
		};

		std::array mats {
			Hx(),
			Hy(),
			Hz(),
			Ex(),
			Ey(),
			Ez(),
		};

		for(auto&& p: std::views::zip(names, mats))
			co_yield p;
	}

	void initCoefs(vk::CommandBuffer commandBuffer)
	{
		//TODO
	}

	void updateH(vk::CommandBuffer commandBuffer)
	{
		//TODO
	}

	void updateE(vk::CommandBuffer commandBuffer)
	{
		//TODO
	}

	void gauss(vk::CommandBuffer commandBuffer)
	{
		//TODO
	}

	void abc(vk::CommandBuffer commandBuffer)
	{
		//TODO
	}

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

	vulkan::SimpleCommandBuffer commandBuffer;

	void init();
	vulkan::CommandRecorder createCommandRecorder();

};

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
