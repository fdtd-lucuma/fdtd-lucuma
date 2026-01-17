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

export module lucuma.services.backends.vulkan_components:fdtd_data;

import lucuma.legacy_headers.mdspan;
import lucuma.legacy_headers.glm;

import lucuma.utils;
import lucuma.components;
import lucuma.services.vulkan;
import vulkan;
import vk_mem_alloc_hpp;

import std;

import :abc_pipeline;
import :gauss_pipeline;
import :init_coefs_pipeline;
import :update_e_pipeline;
import :update_h_pipeline;
import :utils;

namespace lucuma::services::backends::vulkan_components
{

using namespace lucuma::utils;

export template <typename T>
struct FdtdDataCreateInfo
{
	components::FdtdDataCreateInfo fdtdDataCreateInfo;
	vulkan::Compute& compute;
	vulkan::Allocator& allocator;
};

export template <typename T>
class FdtdData
{
public:
	constexpr static auto HxDimsDelta = svec3Delta(0, -1, -1);
	constexpr static auto HyDimsDelta = svec3Delta(-1, 0, -1);
	constexpr static auto HzDimsDelta = svec3Delta(-1, -1, 0);

	constexpr static auto ExDimsDelta = svec3Delta(-1, 0, 0);
	constexpr static auto EyDimsDelta = svec3Delta(0, -1, 0);
	constexpr static auto EzDimsDelta = svec3Delta(0, 0, -1);

	template <typename extents, typename layout = Kokkos::layout_left>
	using mdspan_t = Kokkos::mdspan<T, extents, layout>;

	template <typename extents, typename layout = Kokkos::layout_left>
	using cmdspan_t = Kokkos::mdspan<const T, extents, layout>;

	using extents_2d_t = Kokkos::dextents<std::size_t, 2>;
	using extents_3d_t = Kokkos::dextents<std::size_t, 3>;

	template <typename layout = Kokkos::layout_left>
	using _mdspan_2d_t = mdspan_t<extents_2d_t, layout>;
	template <typename layout = Kokkos::layout_left>
	using _mdspan_3d_t = mdspan_t<extents_3d_t, layout>;

	template <typename layout = Kokkos::layout_left>
	using _cmdspan_2d_t = cmdspan_t<extents_2d_t, layout>;
	template <typename layout = Kokkos::layout_left>
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

	using create_info_t = FdtdDataCreateInfo<T>;

	using MatrixData = vulkan::Buffer;
private:

	static inline auto toMdspan(vulkan::Buffer& buffer, svec3 paddedDims, svec3 dims)
	{
		return unpad(vmdspan_3d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y, paddedDims.z), dims);
	}

	static inline auto toMdspan(vulkan::Buffer& buffer, svec2 paddedDims, svec2 dims)
	{
		return unpad(vmdspan_2d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y), dims);
	}

	static inline auto toMdspan(const vulkan::Buffer& buffer, svec3 paddedDims, svec3 dims)
	{
		return unpad(vcmdspan_3d_t(buffer.getData<T>().data(), paddedDims.x, paddedDims.y, paddedDims.z), dims);
	}

	static inline auto toMdspan(const vulkan::Buffer& buffer, svec2 paddedDims, svec2 dims)
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
			vmaFlags,
			vk::MemoryPropertyFlagBits::eDeviceLocal
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

	FdtdData(const create_info_t& createInfo):
		workGroupSize(createInfo.compute.getWorkgroupSize(createInfo.fdtdDataCreateInfo.size)),
		size(createInfo.fdtdDataCreateInfo.size),
		paddedSize(padDims(size)),
		gaussPosition(createInfo.fdtdDataCreateInfo.gaussPosition),
		deltaT(createInfo.fdtdDataCreateInfo.deltaT),
		deltaX(createInfo.fdtdDataCreateInfo.deltaX),
		deltaY(createInfo.fdtdDataCreateInfo.deltaY),
		deltaZ(createInfo.fdtdDataCreateInfo.deltaZ),
		imp0(createInfo.fdtdDataCreateInfo.imp0),
		Cr(createInfo.fdtdDataCreateInfo.Cr),
		maxTime(createInfo.fdtdDataCreateInfo.maxTime),
		gaussSigma(createInfo.fdtdDataCreateInfo.gaussSigma),
		HxDims((svec3Delta)size + HxDimsDelta),
		HyDims((svec3Delta)size + HyDimsDelta),
		HzDims((svec3Delta)size + HzDimsDelta),
		ExDims((svec3Delta)size + ExDimsDelta),
		EyDims((svec3Delta)size + EyDimsDelta),
		EzDims((svec3Delta)size + EzDimsDelta),
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
		initCoefPipelines(InitCoefPipelinesCreateInfo<T>{
			.Cr = Cr,
			.Imp0 = imp0,
			.deltaT = deltaT,
			.shaderPath = shaderName<T>("init_coefs"),
			.workGroupSize = workGroupSize,
			.compute = createInfo.compute,
			.Hx = {
				.paddedDims = paddedHxDims,
				.dims = HxDims,
				.Ch = _Chxh,
				.Ce = _Chxe,
				.CM = _CMhx,
				.mu = _mux,
				.delta = deltaX,
			},
			.Hy = {
				.paddedDims = paddedHyDims,
				.dims = HyDims,
				.Ch = _Chyh,
				.Ce = _Chye,
				.CM = _CMhy,
				.mu = _muy,
				.delta = deltaY,
			},
			.Hz = {
				.paddedDims = paddedHzDims,
				.dims = HzDims,
				.Ch = _Chzh,
				.Ce = _Chze,
				.CM = _CMhz,
				.mu = _muz,
				.delta = deltaZ,
			},
			.Ex = {
				.paddedDims = paddedExDims,
				.dims = ExDims,
				.Ch = _Cexe,
				.Ce = _Cexh,
				.CM = _CEEx,
				.mu = _epsx,
				.delta = deltaX,
			},
			.Ey = {
				.paddedDims = paddedEyDims,
				.dims = EyDims,
				.Ch = _Ceye,
				.Ce = _Ceyh,
				.CM = _CEEy,
				.mu = _epsy,
				.delta = deltaY,
			},
			.Ez = {
				.paddedDims = paddedEzDims,
				.dims = EzDims,
				.Ch = _Ceze,
				.Ce = _Cezh,
				.CM = _CEEz,
				.mu = _epsz,
				.delta = deltaZ,
			},
		}),
		updateHPipelines(UpdateHPipelinesCreateInfo<T>{
			.shaderPath = shaderName<T>("update_h"),
			.workGroupSize = workGroupSize,
			.compute = createInfo.compute,
			.deltaT = deltaT,
			.x = {

				.paddedHDims   = paddedHxDims,
				.paddedEc1Dims = paddedEyDims,
				.paddedEc2Dims = paddedEzDims,
				.paddedMu1Dims = paddedHyDims,
				.paddedMu2Dims = paddedHzDims,
				.HDims         = HxDims,

				.Ec1Delta = -EzDimsDelta,
				.Ec2Delta = -EyDimsDelta,

				.Hc  = _Hx,
				.Ch  = _Chxh,
				.Ce  = _Chxe,
				.Ec1 = _Ey,
				.Ec2 = _Ez,
				.mu1 = _muy,
				.mu2 = _muz,
				.delta1 = deltaY,
				.delta2 = deltaZ,

			},
			.y = {

				.paddedHDims   = paddedHyDims,
				.paddedEc1Dims = paddedEzDims,
				.paddedEc2Dims = paddedExDims,
				.paddedMu1Dims = paddedHzDims,
				.paddedMu2Dims = paddedHxDims,
				.HDims         = HyDims,

				.Ec1Delta = -ExDimsDelta,
				.Ec2Delta = -EzDimsDelta,

				.Hc  = _Hy,
				.Ch  = _Chyh,
				.Ce  = _Chye,
				.Ec1 = _Ez,
				.Ec2 = _Ex,
				.mu1 = _muz,
				.mu2 = _mux,
				.delta1 = deltaZ,
				.delta2 = deltaX,

			},
			.z = {

				.paddedHDims   = paddedHzDims,
				.paddedEc1Dims = paddedExDims,
				.paddedEc2Dims = paddedEyDims,
				.paddedMu1Dims = paddedHxDims,
				.paddedMu2Dims = paddedHyDims,
				.HDims         = HzDims,

				.Ec1Delta = -EyDimsDelta,
				.Ec2Delta = -ExDimsDelta,

				.Hc  = _Hz,
				.Ch  = _Chzh,
				.Ce  = _Chze,
				.Ec1 = _Ex,
				.Ec2 = _Ey,
				.mu1 = _mux,
				.mu2 = _muy,
				.delta1 = deltaX,
				.delta2 = deltaY,

			},
		}),
		updateEPipelines(UpdateEPipelinesCreateInfo<T>{
			.shaderPath = shaderName<T>("update_e"),
			.workGroupSize = workGroupSize,
			.dims = size - svec3(1),
			.compute = createInfo.compute,
			.deltaT = deltaT,
			.x = {

				.paddedEDims    = paddedExDims,
				.paddedHc1Dims  = paddedHzDims,
				.paddedHc2Dims  = paddedHyDims,
				.paddedEps1Dims = paddedEzDims,
				.paddedEps2Dims = paddedEyDims,
				.start          = -HxDimsDelta,

				.Hc1Delta = EyDimsDelta,
				.Hc2Delta = EzDimsDelta,

				.Ec  = _Ex,
				.Ce  = _Cexe,
				.Ch  = _Cexh,
				.Hc1 = _Hz,
				.Hc2 = _Hy,
				.eps1 = _epsz,
				.eps2 = _epsy,
				.delta1 = deltaZ,
				.delta2 = deltaY,

			},
			.y = {

				.paddedEDims   = paddedEyDims,
				.paddedHc1Dims = paddedHxDims,
				.paddedHc2Dims = paddedHzDims,
				.paddedEps1Dims = paddedExDims,
				.paddedEps2Dims = paddedEzDims,
				.start         = -HyDimsDelta,

				.Hc1Delta = EzDimsDelta,
				.Hc2Delta = ExDimsDelta,

				.Ec  = _Ey,
				.Ce  = _Ceye,
				.Ch  = _Ceyh,
				.Hc1 = _Hx,
				.Hc2 = _Hz,
				.eps1 = _epsx,
				.eps2 = _epsz,
				.delta1 = deltaX,
				.delta2 = deltaZ,

			},
			.z = {

				.paddedEDims    = paddedEzDims,
				.paddedHc1Dims  = paddedHyDims,
				.paddedHc2Dims  = paddedHxDims,
				.paddedEps1Dims = paddedEyDims,
				.paddedEps2Dims = paddedExDims,
				.start          = -HzDimsDelta,

				.Hc1Delta = ExDimsDelta,
				.Hc2Delta = EyDimsDelta,

				.Ec  = _Ez,
				.Ce  = _Ceze,
				.Ch  = _Cezh,
				.Hc1 = _Hy,
				.Hc2 = _Hx,
				.eps1 = _epsy,
				.eps2 = _epsx,
				.delta1 = deltaY,
				.delta2 = deltaX,

			},
		}),
		gaussPipeline(GaussPipelineCreateInfo<T>{
			.paddedDims = paddedExDims,
			.Ec         = _Ex,
			.shaderPath = shaderName<T>("gauss"),
			.compute    = createInfo.compute,
		}),
		abcPipelines(AbcPipelinesCreateInfo<T>{
			.Cr = Cr,
			.paddedDims = paddedSize,
			.dims = size,
			.shaderPath = shaderName<T>("abc"),
			.workGroupSize = workGroupSize,
			.compute = createInfo.compute,
			.x = {
				.paddedEc1Dims = paddedEyDims,
				.paddedEc2Dims = paddedEzDims,
				.paddedec1Dims = paddedEyxDims,
				.paddedec2Dims = paddedEzxDims,

				.Ec1  = _Ey,
				.Ec2  = _Ez,
				.mu   = _muxR,
				.eps  = _epsxR,
				.ec10 = _eyx0,
				.ec11 = _eyx1,
				.ec20 = _ezx0,
				.ec21 = _ezx1,
			},
			.y = {
				.paddedEc1Dims = paddedExDims,
				.paddedEc2Dims = paddedEzDims,
				.paddedec1Dims = paddedExyDims,
				.paddedec2Dims = paddedEzyDims,

				.Ec1  = _Ex,
				.Ec2  = _Ez,
				.mu   = _muyR,
				.eps  = _epsyR,
				.ec10 = _exy0,
				.ec11 = _exy1,
				.ec20 = _ezy0,
				.ec21 = _ezy1,
			},
			.z = {
				.paddedEc1Dims = paddedExDims,
				.paddedEc2Dims = paddedEyDims,
				.paddedec1Dims = paddedExzDims,
				.paddedec2Dims = paddedEyzDims,

				.Ec1  = _Ex,
				.Ec2  = _Ey,
				.mu   = _muzR,
				.eps  = _epszR,
				.ec10 = _exz0,
				.ec11 = _exz1,
				.ec20 = _eyz0,
				.ec21 = _eyz1,
			},
		})
	{
	}

	const svec3 workGroupSize;
	const svec3 size;
	const svec3 paddedSize;
	const svec3 gaussPosition;
	const T deltaT;
	const T deltaX;
	const T deltaY;
	const T deltaZ;
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

	InitCoefPipelines<T> initCoefPipelines;
	UpdateHPipelines<T>  updateHPipelines;
	UpdateEPipelines<T>  updateEPipelines;
	GaussPipeline<T>     gaussPipeline;
	AbcPipelines<T>      abcPipelines;

public:

	cmdspan_3d_t Hx()    const { return toMdspan(_Hx,    paddedHxDims, HxDims);  }
	cmdspan_3d_t Hy()    const { return toMdspan(_Hy,    paddedHyDims, HyDims);  }
	cmdspan_3d_t Hz()    const { return toMdspan(_Hz,    paddedHzDims, HzDims);  }
	cmdspan_3d_t Chxh()  const { return toMdspan(_Chxh,  paddedHxDims, HxDims);  }
	cmdspan_3d_t Chyh()  const { return toMdspan(_Chyh,  paddedHyDims, HyDims);  }
	cmdspan_3d_t Chzh()  const { return toMdspan(_Chzh,  paddedHzDims, HzDims);  }
	cmdspan_3d_t Chxe()  const { return toMdspan(_Chxe,  paddedHxDims, HxDims);  }
	cmdspan_3d_t Chye()  const { return toMdspan(_Chye,  paddedHyDims, HyDims);  }
	cmdspan_3d_t Chze()  const { return toMdspan(_Chze,  paddedHzDims, HzDims);  }
	cmdspan_3d_t CMhx()  const { return toMdspan(_CMhx,  paddedHxDims, HxDims);  }
	cmdspan_3d_t CMhy()  const { return toMdspan(_CMhy,  paddedHyDims, HyDims);  }
	cmdspan_3d_t CMhz()  const { return toMdspan(_CMhz,  paddedHzDims, HzDims);  }
	cmdspan_3d_t mux()   const { return toMdspan(_mux,   paddedHxDims, HxDims);  }
	cmdspan_3d_t muy()   const { return toMdspan(_muy,   paddedHyDims, HyDims);  }
	cmdspan_3d_t muz()   const { return toMdspan(_muz,   paddedHzDims, HzDims);  }
	cmdspan_3d_t muxR()  const { return toMdspan(_muxR,  paddedSize, size);    }
	cmdspan_3d_t muyR()  const { return toMdspan(_muyR,  paddedSize, size);    }
	cmdspan_3d_t muzR()  const { return toMdspan(_muzR,  paddedSize, size);    }
	cmdspan_3d_t Ex()    const { return toMdspan(_Ex,    paddedExDims, ExDims);  }
	cmdspan_3d_t Ey()    const { return toMdspan(_Ey,    paddedEyDims, EyDims);  }
	cmdspan_3d_t Ez()    const { return toMdspan(_Ez,    paddedEzDims, EzDims);  }
	cmdspan_3d_t Cexe()  const { return toMdspan(_Cexe,  paddedExDims, ExDims);  }
	cmdspan_3d_t Ceye()  const { return toMdspan(_Ceye,  paddedEyDims, EyDims);  }
	cmdspan_3d_t Ceze()  const { return toMdspan(_Ceze,  paddedEzDims, EzDims);  }
	cmdspan_3d_t Cexh()  const { return toMdspan(_Cexh,  paddedExDims, ExDims);  }
	cmdspan_3d_t Ceyh()  const { return toMdspan(_Ceyh,  paddedEyDims, EyDims);  }
	cmdspan_3d_t Cezh()  const { return toMdspan(_Cezh,  paddedEzDims, EzDims);  }
	cmdspan_3d_t CEEx()  const { return toMdspan(_CEEx,  paddedExDims, ExDims);  }
	cmdspan_3d_t CEEy()  const { return toMdspan(_CEEy,  paddedEyDims, EyDims);  }
	cmdspan_3d_t CEEz()  const { return toMdspan(_CEEz,  paddedEzDims, EzDims);  }
	cmdspan_3d_t epsx()  const { return toMdspan(_epsx,  paddedExDims, ExDims);  }
	cmdspan_3d_t epsy()  const { return toMdspan(_epsy,  paddedEyDims, EyDims);  }
	cmdspan_3d_t epsz()  const { return toMdspan(_epsz,  paddedEzDims, EzDims);  }
	cmdspan_3d_t epsxR() const { return toMdspan(_epsxR, paddedSize, size);    }
	cmdspan_3d_t epsyR() const { return toMdspan(_epsyR, paddedSize, size);    }
	cmdspan_3d_t epszR() const { return toMdspan(_epszR, paddedSize, size);    }

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

	std::generator<std::tuple<const char*, cmdspan_3d_t>> chZippedFields() const {
		static constexpr std::array names {
			"Chxh()",
			"Chyh()",
			"Chzh()",
			"Chxe()",
			"Chye()",
			"Chze()",
		};

		std::array mats {
			Chxh(),
			Chyh(),
			Chzh(),
			Chxe(),
			Chye(),
			Chze(),
		};

		for(auto&& p: std::views::zip(names, mats))
			co_yield p;
	}

	std::generator<std::tuple<const char*, cmdspan_3d_t>> ceZippedFields() const {
		static constexpr std::array names {
			"Cexe()",
			"Ceye()",
			"Ceze()",
			"Cexh()",
			"Ceyh()",
			"Cezh()",
		};

		std::array mats {
			Cexe(),
			Ceye(),
			Ceze(),
			Cexh(),
			Ceyh(),
			Cezh(),
		};

		for(auto&& p: std::views::zip(names, mats))
			co_yield p;
	}

	void initCoefs(vk::CommandBuffer commandBuffer)
	{
		initCoefPipelines.dispatch(commandBuffer);
	}

	void updateH(vk::CommandBuffer commandBuffer)
	{
		updateHPipelines.dispatch(commandBuffer);
	}

	void updateE(vk::CommandBuffer commandBuffer)
	{
		updateEPipelines.dispatch(commandBuffer);
	}

	void gauss(vk::CommandBuffer commandBuffer)
	{
		gaussPipeline.dispatch(commandBuffer, gaussPosition, time, gaussSigma);
	}

	void abc(vk::CommandBuffer commandBuffer)
	{
		abcPipelines.dispatch(commandBuffer);
	}

};

}
