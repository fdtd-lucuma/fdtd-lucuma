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

export module lucuma.services.backends.vulkan_components:fdtd_data;

import lucuma.legacy_headers.entt;
import lucuma.legacy_headers.glm;
import lucuma.legacy_headers.mdspan;

import lucuma.utils;
import lucuma.components;
import lucuma.services.vulkan;
import vulkan_hpp;
import vk_mem_alloc;

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

	template <components::FloatFileReader<T> floater_t>
	static void readInto(floater_t& reader, mdspan_3d_t matrix, const std::filesystem::path& path)
	{
		if(!std::filesystem::exists(path))
			return;

		auto fileFloats = reader.readIntoFloats(path);

		T f;

		// TODO: Use an staging buffer
		for(std::size_t i = 0; i < matrix.extent(0); i++)
		{
			for(std::size_t j = 0; j < matrix.extent(1); j++)
			{
				for(std::size_t k = 0; k < matrix.extent(2); k++)
				{
					if(!fileFloats.readOne(f))
						return;

					matrix[i,j,k] = f;
				}
			}
		}
	}

	template <components::FloatFileReader<T> floater_t>
	static void readInto(floater_t& reader, mdspan_2d_t matrix, const std::filesystem::path& path)
	{
		if(!std::filesystem::exists(path))
			return;

		auto fileFloats = reader.readIntoFloats(path);

		T f;

		// TODO: Use an staging buffer
		for(std::size_t i = 0; i < matrix.extent(0); i++)
		{
			for(std::size_t j = 0; j < matrix.extent(1); j++)
			{
				if(!fileFloats.readOne(f))
					return;

				matrix[i,j] = f;
			}
		}
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
			},
			.Hy = {
				.paddedDims = paddedHyDims,
				.dims = HyDims,
				.Ch = _Chyh,
				.Ce = _Chye,
				.CM = _CMhy,
				.mu = _muy,
			},
			.Hz = {
				.paddedDims = paddedHzDims,
				.dims = HzDims,
				.Ch = _Chzh,
				.Ce = _Chze,
				.CM = _CMhz,
				.mu = _muz,
			},
			.Ex = {
				.paddedDims = paddedExDims,
				.dims = ExDims,
				.Ch = _Cexe,
				.Ce = _Cexh,
				.CM = _CEEx,
				.mu = _epsx,
			},
			.Ey = {
				.paddedDims = paddedEyDims,
				.dims = EyDims,
				.Ch = _Ceye,
				.Ce = _Ceyh,
				.CM = _CEEy,
				.mu = _epsy,
			},
			.Ez = {
				.paddedDims = paddedEzDims,
				.dims = EzDims,
				.Ch = _Ceze,
				.Ce = _Cezh,
				.CM = _CEEz,
				.mu = _epsz,
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
				.HDims         = HxDims,

				.Ec1Delta = -EzDimsDelta,
				.Ec2Delta = -EyDimsDelta,

				.Hc  = _Hx,
				.Ch  = _Chxh,
				.Ce  = _Chxe,
				.Ec1 = _Ey,
				.Ec2 = _Ez,
				.delta1 = deltaY,
				.delta2 = deltaZ,

			},
			.y = {

				.paddedHDims   = paddedHyDims,
				.paddedEc1Dims = paddedEzDims,
				.paddedEc2Dims = paddedExDims,
				.HDims         = HyDims,

				.Ec1Delta = -ExDimsDelta,
				.Ec2Delta = -EzDimsDelta,

				.Hc  = _Hy,
				.Ch  = _Chyh,
				.Ce  = _Chye,
				.Ec1 = _Ez,
				.Ec2 = _Ex,
				.delta1 = deltaZ,
				.delta2 = deltaX,

			},
			.z = {

				.paddedHDims   = paddedHzDims,
				.paddedEc1Dims = paddedExDims,
				.paddedEc2Dims = paddedEyDims,
				.HDims         = HzDims,

				.Ec1Delta = -EyDimsDelta,
				.Ec2Delta = -ExDimsDelta,

				.Hc  = _Hz,
				.Ch  = _Chzh,
				.Ce  = _Chze,
				.Ec1 = _Ex,
				.Ec2 = _Ey,
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
				.start          = -HxDimsDelta,

				.Hc1Delta = EyDimsDelta,
				.Hc2Delta = EzDimsDelta,

				.Ec  = _Ex,
				.Ce  = _Cexe,
				.Ch  = _Cexh,
				.Hc1 = _Hz,
				.Hc2 = _Hy,
				.delta1 = deltaZ,
				.delta2 = deltaY,

			},
			.y = {

				.paddedEDims   = paddedEyDims,
				.paddedHc1Dims = paddedHxDims,
				.paddedHc2Dims = paddedHzDims,
				.start         = -HyDimsDelta,

				.Hc1Delta = EzDimsDelta,
				.Hc2Delta = ExDimsDelta,

				.Ec  = _Ey,
				.Ce  = _Ceye,
				.Ch  = _Ceyh,
				.Hc1 = _Hx,
				.Hc2 = _Hz,
				.delta1 = deltaX,
				.delta2 = deltaZ,

			},
			.z = {

				.paddedEDims    = paddedEzDims,
				.paddedHc1Dims  = paddedHyDims,
				.paddedHc2Dims  = paddedHxDims,
				.start          = -HzDimsDelta,

				.Hc1Delta = ExDimsDelta,
				.Hc2Delta = EyDimsDelta,

				.Ec  = _Ez,
				.Ce  = _Ceze,
				.Ch  = _Cezh,
				.Hc1 = _Hy,
				.Hc2 = _Hx,
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
		components::fillRegistryWithSources<T>(privateRegistry, createInfo.fdtdDataCreateInfo);
	}

	template <components::FloatFileReader<T> floater_t>
	static FdtdData<T> make(create_info_t& createInfo, floater_t& reader)
	{
		FdtdData<T> result(createInfo);

		readInto(reader, result.Hx(),    createInfo.fdtdDataCreateInfo.Hx0);
		readInto(reader, result.Hy(),    createInfo.fdtdDataCreateInfo.Hy0);
		readInto(reader, result.Hz(),    createInfo.fdtdDataCreateInfo.Hz0);
		readInto(reader, result.Chxh(),  createInfo.fdtdDataCreateInfo.Chxh0);
		readInto(reader, result.Chyh(),  createInfo.fdtdDataCreateInfo.Chyh0);
		readInto(reader, result.Chzh(),  createInfo.fdtdDataCreateInfo.Chzh0);
		readInto(reader, result.Chxe(),  createInfo.fdtdDataCreateInfo.Chxe0);
		readInto(reader, result.Chye(),  createInfo.fdtdDataCreateInfo.Chye0);
		readInto(reader, result.Chze(),  createInfo.fdtdDataCreateInfo.Chze0);
		readInto(reader, result.CMhx(),  createInfo.fdtdDataCreateInfo.CMhx0);
		readInto(reader, result.CMhy(),  createInfo.fdtdDataCreateInfo.CMhy0);
		readInto(reader, result.CMhz(),  createInfo.fdtdDataCreateInfo.CMhz0);
		readInto(reader, result.mux(),   createInfo.fdtdDataCreateInfo.mux0);
		readInto(reader, result.muy(),   createInfo.fdtdDataCreateInfo.muy0);
		readInto(reader, result.muz(),   createInfo.fdtdDataCreateInfo.muz0);
		readInto(reader, result.muxR(),  createInfo.fdtdDataCreateInfo.muxR0);
		readInto(reader, result.muyR(),  createInfo.fdtdDataCreateInfo.muyR0);
		readInto(reader, result.muzR(),  createInfo.fdtdDataCreateInfo.muzR0);
		readInto(reader, result.Ex(),    createInfo.fdtdDataCreateInfo.Ex0);
		readInto(reader, result.Ey(),    createInfo.fdtdDataCreateInfo.Ey0);
		readInto(reader, result.Ez(),    createInfo.fdtdDataCreateInfo.Ez0);
		readInto(reader, result.Cexe(),  createInfo.fdtdDataCreateInfo.Cexe0);
		readInto(reader, result.Ceye(),  createInfo.fdtdDataCreateInfo.Ceye0);
		readInto(reader, result.Ceze(),  createInfo.fdtdDataCreateInfo.Ceze0);
		readInto(reader, result.Cexh(),  createInfo.fdtdDataCreateInfo.Cexh0);
		readInto(reader, result.Ceyh(),  createInfo.fdtdDataCreateInfo.Ceyh0);
		readInto(reader, result.Cezh(),  createInfo.fdtdDataCreateInfo.Cezh0);
		readInto(reader, result.CEEx(),  createInfo.fdtdDataCreateInfo.CEEx0);
		readInto(reader, result.CEEy(),  createInfo.fdtdDataCreateInfo.CEEy0);
		readInto(reader, result.CEEz(),  createInfo.fdtdDataCreateInfo.CEEz0);
		readInto(reader, result.epsx(),  createInfo.fdtdDataCreateInfo.epsx0);
		readInto(reader, result.epsy(),  createInfo.fdtdDataCreateInfo.epsy0);
		readInto(reader, result.epsz(),  createInfo.fdtdDataCreateInfo.epsz0);
		readInto(reader, result.epsxR(), createInfo.fdtdDataCreateInfo.epsxR0);
		readInto(reader, result.epsyR(), createInfo.fdtdDataCreateInfo.epsyR0);
		readInto(reader, result.epszR(), createInfo.fdtdDataCreateInfo.epszR0);
		readInto(reader, result.eyx0(),  createInfo.fdtdDataCreateInfo.eyx00);
		readInto(reader, result.ezx0(),  createInfo.fdtdDataCreateInfo.ezx00);
		readInto(reader, result.eyx1(),  createInfo.fdtdDataCreateInfo.eyx10);
		readInto(reader, result.ezx1(),  createInfo.fdtdDataCreateInfo.ezx10);
		readInto(reader, result.exy0(),  createInfo.fdtdDataCreateInfo.exy00);
		readInto(reader, result.ezy0(),  createInfo.fdtdDataCreateInfo.ezy00);
		readInto(reader, result.exy1(),  createInfo.fdtdDataCreateInfo.exy10);
		readInto(reader, result.ezy1(),  createInfo.fdtdDataCreateInfo.ezy10);
		readInto(reader, result.exz0(),  createInfo.fdtdDataCreateInfo.exz00);
		readInto(reader, result.eyz0(),  createInfo.fdtdDataCreateInfo.eyz00);
		readInto(reader, result.exz1(),  createInfo.fdtdDataCreateInfo.exz10);
		readInto(reader, result.eyz1(),  createInfo.fdtdDataCreateInfo.eyz10);

		return result;
	}

	svec3 workGroupSize;
	svec3 size;
	svec3 paddedSize;
	svec3 gaussPosition;
	T deltaT;
	T deltaX;
	T deltaY;
	T deltaZ;
	T imp0;
	T Cr;

	unsigned int maxTime;

private:
	unsigned int time = 0;
	T gaussSigma;

	// Magnetic field dimentions

	svec3 HxDims;
	svec3 HyDims;
	svec3 HzDims;

	// Electric field dimentions

	svec3 ExDims;
	svec3 EyDims;
	svec3 EzDims;

	// ABC dimentions

	svec2 eyxDims;
	svec2 ezxDims;

	svec2 exyDims;
	svec2 ezyDims;

	svec2 exzDims;
	svec2 eyzDims;

	// Padded Magnetic field dimentions

	svec3 paddedHxDims;
	svec3 paddedHyDims;
	svec3 paddedHzDims;

	// Padded Electric field dimentions

	svec3 paddedExDims;
	svec3 paddedEyDims;
	svec3 paddedEzDims;

	// Padded ABC dimentions

	svec2 paddedEyxDims;
	svec2 paddedEzxDims;

	svec2 paddedExyDims;
	svec2 paddedEzyDims;

	svec2 paddedExzDims;
	svec2 paddedEyzDims;

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

	entt::registry privateRegistry;

public:

	cmdspan_3d_t Hx()    const { return toMdspan(_Hx,    paddedHxDims,  HxDims);  }
	cmdspan_3d_t Hy()    const { return toMdspan(_Hy,    paddedHyDims,  HyDims);  }
	cmdspan_3d_t Hz()    const { return toMdspan(_Hz,    paddedHzDims,  HzDims);  }
	cmdspan_3d_t Chxh()  const { return toMdspan(_Chxh,  paddedHxDims,  HxDims);  }
	cmdspan_3d_t Chyh()  const { return toMdspan(_Chyh,  paddedHyDims,  HyDims);  }
	cmdspan_3d_t Chzh()  const { return toMdspan(_Chzh,  paddedHzDims,  HzDims);  }
	cmdspan_3d_t Chxe()  const { return toMdspan(_Chxe,  paddedHxDims,  HxDims);  }
	cmdspan_3d_t Chye()  const { return toMdspan(_Chye,  paddedHyDims,  HyDims);  }
	cmdspan_3d_t Chze()  const { return toMdspan(_Chze,  paddedHzDims,  HzDims);  }
	cmdspan_3d_t CMhx()  const { return toMdspan(_CMhx,  paddedHxDims,  HxDims);  }
	cmdspan_3d_t CMhy()  const { return toMdspan(_CMhy,  paddedHyDims,  HyDims);  }
	cmdspan_3d_t CMhz()  const { return toMdspan(_CMhz,  paddedHzDims,  HzDims);  }
	cmdspan_3d_t mux()   const { return toMdspan(_mux,   paddedHxDims,  HxDims);  }
	cmdspan_3d_t muy()   const { return toMdspan(_muy,   paddedHyDims,  HyDims);  }
	cmdspan_3d_t muz()   const { return toMdspan(_muz,   paddedHzDims,  HzDims);  }
	cmdspan_3d_t muxR()  const { return toMdspan(_muxR,  paddedSize,    size);    }
	cmdspan_3d_t muyR()  const { return toMdspan(_muyR,  paddedSize,    size);    }
	cmdspan_3d_t muzR()  const { return toMdspan(_muzR,  paddedSize,    size);    }
	cmdspan_3d_t Ex()    const { return toMdspan(_Ex,    paddedExDims,  ExDims);  }
	cmdspan_3d_t Ey()    const { return toMdspan(_Ey,    paddedEyDims,  EyDims);  }
	cmdspan_3d_t Ez()    const { return toMdspan(_Ez,    paddedEzDims,  EzDims);  }
	cmdspan_3d_t Cexe()  const { return toMdspan(_Cexe,  paddedExDims,  ExDims);  }
	cmdspan_3d_t Ceye()  const { return toMdspan(_Ceye,  paddedEyDims,  EyDims);  }
	cmdspan_3d_t Ceze()  const { return toMdspan(_Ceze,  paddedEzDims,  EzDims);  }
	cmdspan_3d_t Cexh()  const { return toMdspan(_Cexh,  paddedExDims,  ExDims);  }
	cmdspan_3d_t Ceyh()  const { return toMdspan(_Ceyh,  paddedEyDims,  EyDims);  }
	cmdspan_3d_t Cezh()  const { return toMdspan(_Cezh,  paddedEzDims,  EzDims);  }
	cmdspan_3d_t CEEx()  const { return toMdspan(_CEEx,  paddedExDims,  ExDims);  }
	cmdspan_3d_t CEEy()  const { return toMdspan(_CEEy,  paddedEyDims,  EyDims);  }
	cmdspan_3d_t CEEz()  const { return toMdspan(_CEEz,  paddedEzDims,  EzDims);  }
	cmdspan_3d_t epsx()  const { return toMdspan(_epsx,  paddedExDims,  ExDims);  }
	cmdspan_3d_t epsy()  const { return toMdspan(_epsy,  paddedEyDims,  EyDims);  }
	cmdspan_3d_t epsz()  const { return toMdspan(_epsz,  paddedEzDims,  EzDims);  }
	cmdspan_3d_t epsxR() const { return toMdspan(_epsxR, paddedSize,    size);    }
	cmdspan_3d_t epsyR() const { return toMdspan(_epsyR, paddedSize,    size);    }
	cmdspan_3d_t epszR() const { return toMdspan(_epszR, paddedSize,    size);    }
	cmdspan_2d_t eyx0()  const { return toMdspan(_eyx0,  paddedEyxDims, eyxDims); }
	cmdspan_2d_t ezx0()  const { return toMdspan(_ezx0,  paddedEzxDims, ezxDims); }
	cmdspan_2d_t eyx1()  const { return toMdspan(_eyx1,  paddedEyxDims, eyxDims); }
	cmdspan_2d_t ezx1()  const { return toMdspan(_ezx1,  paddedEzxDims, ezxDims); }
	cmdspan_2d_t exy0()  const { return toMdspan(_exy0,  paddedExyDims, exyDims); }
	cmdspan_2d_t ezy0()  const { return toMdspan(_ezy0,  paddedEzyDims, ezyDims); }
	cmdspan_2d_t exy1()  const { return toMdspan(_exy1,  paddedExyDims, exyDims); }
	cmdspan_2d_t ezy1()  const { return toMdspan(_ezy1,  paddedEzyDims, ezyDims); }
	cmdspan_2d_t exz0()  const { return toMdspan(_exz0,  paddedExzDims, exzDims); }
	cmdspan_2d_t eyz0()  const { return toMdspan(_eyz0,  paddedEyzDims, eyzDims); }
	cmdspan_2d_t exz1()  const { return toMdspan(_exz1,  paddedExzDims, exzDims); }
	cmdspan_2d_t eyz1()  const { return toMdspan(_eyz1,  paddedEyzDims, eyzDims); }

	mdspan_3d_t Hx()    { return toMdspan(_Hx,    paddedHxDims,  HxDims);  }
	mdspan_3d_t Hy()    { return toMdspan(_Hy,    paddedHyDims,  HyDims);  }
	mdspan_3d_t Hz()    { return toMdspan(_Hz,    paddedHzDims,  HzDims);  }
	mdspan_3d_t Chxh()  { return toMdspan(_Chxh,  paddedHxDims,  HxDims);  }
	mdspan_3d_t Chyh()  { return toMdspan(_Chyh,  paddedHyDims,  HyDims);  }
	mdspan_3d_t Chzh()  { return toMdspan(_Chzh,  paddedHzDims,  HzDims);  }
	mdspan_3d_t Chxe()  { return toMdspan(_Chxe,  paddedHxDims,  HxDims);  }
	mdspan_3d_t Chye()  { return toMdspan(_Chye,  paddedHyDims,  HyDims);  }
	mdspan_3d_t Chze()  { return toMdspan(_Chze,  paddedHzDims,  HzDims);  }
	mdspan_3d_t CMhx()  { return toMdspan(_CMhx,  paddedHxDims,  HxDims);  }
	mdspan_3d_t CMhy()  { return toMdspan(_CMhy,  paddedHyDims,  HyDims);  }
	mdspan_3d_t CMhz()  { return toMdspan(_CMhz,  paddedHzDims,  HzDims);  }
	mdspan_3d_t mux()   { return toMdspan(_mux,   paddedHxDims,  HxDims);  }
	mdspan_3d_t muy()   { return toMdspan(_muy,   paddedHyDims,  HyDims);  }
	mdspan_3d_t muz()   { return toMdspan(_muz,   paddedHzDims,  HzDims);  }
	mdspan_3d_t muxR()  { return toMdspan(_muxR,  paddedSize,    size);    }
	mdspan_3d_t muyR()  { return toMdspan(_muyR,  paddedSize,    size);    }
	mdspan_3d_t muzR()  { return toMdspan(_muzR,  paddedSize,    size);    }
	mdspan_3d_t Ex()    { return toMdspan(_Ex,    paddedExDims,  ExDims);  }
	mdspan_3d_t Ey()    { return toMdspan(_Ey,    paddedEyDims,  EyDims);  }
	mdspan_3d_t Ez()    { return toMdspan(_Ez,    paddedEzDims,  EzDims);  }
	mdspan_3d_t Cexe()  { return toMdspan(_Cexe,  paddedExDims,  ExDims);  }
	mdspan_3d_t Ceye()  { return toMdspan(_Ceye,  paddedEyDims,  EyDims);  }
	mdspan_3d_t Ceze()  { return toMdspan(_Ceze,  paddedEzDims,  EzDims);  }
	mdspan_3d_t Cexh()  { return toMdspan(_Cexh,  paddedExDims,  ExDims);  }
	mdspan_3d_t Ceyh()  { return toMdspan(_Ceyh,  paddedEyDims,  EyDims);  }
	mdspan_3d_t Cezh()  { return toMdspan(_Cezh,  paddedEzDims,  EzDims);  }
	mdspan_3d_t CEEx()  { return toMdspan(_CEEx,  paddedExDims,  ExDims);  }
	mdspan_3d_t CEEy()  { return toMdspan(_CEEy,  paddedEyDims,  EyDims);  }
	mdspan_3d_t CEEz()  { return toMdspan(_CEEz,  paddedEzDims,  EzDims);  }
	mdspan_3d_t epsx()  { return toMdspan(_epsx,  paddedExDims,  ExDims);  }
	mdspan_3d_t epsy()  { return toMdspan(_epsy,  paddedEyDims,  EyDims);  }
	mdspan_3d_t epsz()  { return toMdspan(_epsz,  paddedEzDims,  EzDims);  }
	mdspan_3d_t epsxR() { return toMdspan(_epsxR, paddedSize,    size);    }
	mdspan_3d_t epsyR() { return toMdspan(_epsyR, paddedSize,    size);    }
	mdspan_3d_t epszR() { return toMdspan(_epszR, paddedSize,    size);    }
	mdspan_2d_t eyx0()  { return toMdspan(_eyx0,  paddedEyxDims, eyxDims); }
	mdspan_2d_t ezx0()  { return toMdspan(_ezx0,  paddedEzxDims, ezxDims); }
	mdspan_2d_t eyx1()  { return toMdspan(_eyx1,  paddedEyxDims, eyxDims); }
	mdspan_2d_t ezx1()  { return toMdspan(_ezx1,  paddedEzxDims, ezxDims); }
	mdspan_2d_t exy0()  { return toMdspan(_exy0,  paddedExyDims, exyDims); }
	mdspan_2d_t ezy0()  { return toMdspan(_ezy0,  paddedEzyDims, ezyDims); }
	mdspan_2d_t exy1()  { return toMdspan(_exy1,  paddedExyDims, exyDims); }
	mdspan_2d_t ezy1()  { return toMdspan(_ezy1,  paddedEzyDims, ezyDims); }
	mdspan_2d_t exz0()  { return toMdspan(_exz0,  paddedExzDims, exzDims); }
	mdspan_2d_t eyz0()  { return toMdspan(_eyz0,  paddedEyzDims, eyzDims); }
	mdspan_2d_t exz1()  { return toMdspan(_exz1,  paddedExzDims, exzDims); }
	mdspan_2d_t eyz1()  { return toMdspan(_eyz1,  paddedEyzDims, eyzDims); }

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

	std::vector<std::pair<std::string, cmdspan_3d_t>> zippedFields() const {
		return {
			{"Hx",Hx()},
			{"Hy",Hy()},
			{"Hz",Hz()},
			{"Ex",Ex()},
			{"Ey",Ey()},
			{"Ez",Ez()},
		};
	}

	std::vector<std::pair<std::string, cmdspan_3d_t>> chZippedFields() const {
		return {
			{"Chxh()",Chxh()},
			{"Chyh()",Chyh()},
			{"Chzh()",Chzh()},
			{"Chxe()",Chxe()},
			{"Chye()",Chye()},
			{"Chze()",Chze()},
		};
	}

	std::vector<std::pair<std::string, cmdspan_3d_t>> ceZippedFields() const {
		return {
			{"Cexe()",Cexe()},
			{"Ceye()",Ceye()},
			{"Ceze()",Ceze()},
			{"Cexh()",Cexh()},
			{"Ceyh()",Ceyh()},
			{"Cezh()",Cezh()},
		};
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
