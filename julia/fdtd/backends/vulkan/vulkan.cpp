// No direct Julia counterpart — mirrors fdtd-lucuma/src/services/backends/vulkan.cpp.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fdtd/backends/vulkan/vulkan.hpp"

#include <array>
#include <cstring>
#include <vector>

#include "fdtd/backends/vulkan/context.hpp"
#include "fdtd/backends/vulkan/pipeline.hpp"
#include "fdtd/fw_sim.hpp"     // DFTSampleTimes / DFTPhase
#include "fdtd/params.hpp"     // FillCoeffs*

#include "update_h_comp_spv.h"
#include "update_e_comp_spv.h"
#include "inject_e_comp_spv.h"
#include "inject_m_comp_spv.h"
#include "accum_dft_comp_spv.h"

namespace lucuma::julia {

using vk::Buffer;
using vk::ComputePipeline;
using vk::Context;

namespace {
inline uint32_t ceilDiv(int a, int b) {
	return static_cast<uint32_t>((a + b - 1) / b);
}
}  // namespace

template <class T>
struct Vulkan<T>::Impl {
	Context ctx;

	int Nx = 0, Ny = 0, Nz = 0;
	std::size_t cells = 0;
	double dt = 0.0;
	float dx = 0, dy = 0, dz = 0;
	std::vector<double> freqs;

	Buffer Ex, Ey, Ez, Hx, Hy, Hz;
	Buffer Chh, Che, Cee, Ceh;
	Grid3<float> chhCpu, cheCpu, ceeCpu, cehCpu;  // for opt-box refill + reupload

	// per frequency: {E1,E2,E3,H1,H2,H3} as vec2 (complex<float>)
	std::vector<std::array<Buffer, 6>> dft;

	struct Src {
		int axis, efixed0, mfixed0, r1_lo0, r2_lo0, size1, size2, nfreqs;
		float scale;  // J0 * envelope(==1) * inv_delta_normal
		double time_origin;
		Buffer Jx, Jy, Jz, Mx, My, Mz, omegas;
	};
	std::vector<Src> srcs;

	std::unique_ptr<ComputePipeline> uh, ue;
	std::vector<std::unique_ptr<ComputePipeline>> ie, im;   // per source
	std::vector<std::unique_ptr<ComputePipeline>> adft;     // per frequency

	Buffer mk(std::size_t bytes) { return ctx.createBuffer(bytes); }

	void uploadCoeffsWhole() {
		std::memcpy(Chh.mapped, chhCpu.data(), cells * sizeof(float));
		std::memcpy(Che.mapped, cheCpu.data(), cells * sizeof(float));
		std::memcpy(Cee.mapped, ceeCpu.data(), cells * sizeof(float));
		std::memcpy(Ceh.mapped, cehCpu.data(), cells * sizeof(float));
	}
};

// ---------------------------------------------------------------------------

template <class T>
Vulkan<T>::Vulkan() : impl_(std::make_unique<Impl>()) {}

template <class T>
Vulkan<T>::~Vulkan() {
	if (!impl_) return;
	auto& d = *impl_;
	for (Buffer* b : {&d.Ex, &d.Ey, &d.Ez, &d.Hx, &d.Hy, &d.Hz, &d.Chh, &d.Che,
	                  &d.Cee, &d.Ceh})
		d.ctx.destroy(*b);
	for (auto& f : d.dft)
		for (auto& b : f) d.ctx.destroy(b);
	for (auto& s : d.srcs)
		for (Buffer* b : {&s.Jx, &s.Jy, &s.Jz, &s.Mx, &s.My, &s.Mz, &s.omegas})
			d.ctx.destroy(*b);
}

template <class T>
void Vulkan<T>::init(const FDTDParams& p) {
	auto& d = *impl_;
	d.Nx = p.whole_region_sizes[0];
	d.Ny = p.whole_region_sizes[1];
	d.Nz = p.whole_region_sizes[2];
	d.cells = std::size_t(d.Nx) * d.Ny * d.Nz;
	d.dt = p.dt;
	d.dx = float(p.dx);
	d.dy = float(p.dy);
	d.dz = float(p.dz);
	d.freqs = p.freqs;

	const std::size_t fbytes = d.cells * sizeof(float);
	d.Ex = d.mk(fbytes); d.Ey = d.mk(fbytes); d.Ez = d.mk(fbytes);
	d.Hx = d.mk(fbytes); d.Hy = d.mk(fbytes); d.Hz = d.mk(fbytes);
	d.Chh = d.mk(fbytes); d.Che = d.mk(fbytes);
	d.Cee = d.mk(fbytes); d.Ceh = d.mk(fbytes);

	FillCoeffsWholeGrid<float>(d.chhCpu, d.cheCpu, d.ceeCpu, d.cehCpu, p);
	d.uploadCoeffsWhole();

	d.dft.resize(d.freqs.size());
	for (auto& f : d.dft)
		for (auto& b : f) b = d.mk(d.cells * 2 * sizeof(float));

	for (const auto& s : p.sources) {
		typename Impl::Src S;
		S.axis = int(s.axis);  // Axis::X==1
		S.efixed0 = int(s.electric_fixed_index) - 1;
		S.mfixed0 = int(s.magnetic_fixed_index) - 1;
		S.r1_lo0 = int(s.r1_lo) - 1;
		S.r2_lo0 = int(s.r2_lo) - 1;
		S.size1 = s.size1;
		S.size2 = s.size2;
		S.nfreqs = s.nfreqs;
		S.scale = float(s.J0 * s.inv_delta_normal);  // envelope == 1
		S.time_origin = s.time_origin;
		const std::size_t sheet = std::size_t(s.size1) * s.size2 * s.nfreqs;

		auto up = [&](const std::vector<std::complex<double>>& src) {
			Buffer b = d.mk(sheet * 2 * sizeof(float));
			auto* dst = static_cast<float*>(b.mapped);
			for (std::size_t i = 0; i < sheet; ++i) {
				dst[2 * i + 0] = float(src[i].real());
				dst[2 * i + 1] = float(src[i].imag());
			}
			return b;
		};
		S.Jx = up(s.Jx); S.Jy = up(s.Jy); S.Jz = up(s.Jz);
		S.Mx = up(s.Mx); S.My = up(s.My); S.Mz = up(s.Mz);
		S.omegas = d.mk(s.nfreqs * sizeof(float));
		{
			auto* w = static_cast<float*>(S.omegas.mapped);
			for (int n = 0; n < s.nfreqs; ++n) w[n] = float(s.omegas[n]);
		}
		d.srcs.push_back(std::move(S));
	}

	// ---- pipelines ----
	auto spv = [](const uint32_t* a, std::size_t n) {
		return std::span<const uint32_t>(a, n);
	};
	d.uh = std::make_unique<ComputePipeline>(
	    d.ctx, spv(update_h_comp_spv, std::size(update_h_comp_spv)), 8, 32);
	d.uh->bindBuffers(std::array<VkBuffer, 8>{d.Hx.buffer, d.Hy.buffer,
	                                          d.Hz.buffer, d.Ex.buffer,
	                                          d.Ey.buffer, d.Ez.buffer,
	                                          d.Chh.buffer, d.Che.buffer});

	d.ue = std::make_unique<ComputePipeline>(
	    d.ctx, spv(update_e_comp_spv, std::size(update_e_comp_spv)), 8, 32);
	d.ue->bindBuffers(std::array<VkBuffer, 8>{d.Ex.buffer, d.Ey.buffer,
	                                          d.Ez.buffer, d.Hx.buffer,
	                                          d.Hy.buffer, d.Hz.buffer,
	                                          d.Cee.buffer, d.Ceh.buffer});

	for (auto& S : d.srcs) {
		auto p_e = std::make_unique<ComputePipeline>(
		    d.ctx, spv(inject_e_comp_spv, std::size(inject_e_comp_spv)), 8, 64);
		p_e->bindBuffers(std::array<VkBuffer, 8>{d.Ex.buffer, d.Ey.buffer,
		                                         d.Ez.buffer, d.Ceh.buffer,
		                                         S.Jx.buffer, S.Jy.buffer,
		                                         S.Jz.buffer, S.omegas.buffer});
		d.ie.push_back(std::move(p_e));

		auto p_m = std::make_unique<ComputePipeline>(
		    d.ctx, spv(inject_m_comp_spv, std::size(inject_m_comp_spv)), 8, 64);
		p_m->bindBuffers(std::array<VkBuffer, 8>{d.Hx.buffer, d.Hy.buffer,
		                                         d.Hz.buffer, d.Che.buffer,
		                                         S.Mx.buffer, S.My.buffer,
		                                         S.Mz.buffer, S.omegas.buffer});
		d.im.push_back(std::move(p_m));
	}

	for (std::size_t f = 0; f < d.freqs.size(); ++f) {
		auto p_a = std::make_unique<ComputePipeline>(
		    d.ctx, spv(accum_dft_comp_spv, std::size(accum_dft_comp_spv)), 12,
		    32);
		p_a->bindBuffers(std::array<VkBuffer, 12>{
		    d.dft[f][0].buffer, d.dft[f][1].buffer, d.dft[f][2].buffer,
		    d.dft[f][3].buffer, d.dft[f][4].buffer, d.dft[f][5].buffer,
		    d.Ex.buffer, d.Ey.buffer, d.Ez.buffer, d.Hx.buffer, d.Hy.buffer,
		    d.Hz.buffer});
		d.adft.push_back(std::move(p_a));
	}
}

template <class T>
void Vulkan<T>::updateOptRegionCoeffs(const FDTDParams& p) {
	auto& d = *impl_;
	FillCoeffsOptBox<float>(d.chhCpu, d.cheCpu, d.ceeCpu, d.cehCpu, p);
	d.uploadCoeffsWhole();
}

template <class T>
void Vulkan<T>::reset() {
	auto& d = *impl_;
	const std::size_t fb = d.cells * sizeof(float);
	for (Buffer* b : {&d.Ex, &d.Ey, &d.Ez, &d.Hx, &d.Hy, &d.Hz})
		std::memset(b->mapped, 0, fb);
	for (auto& f : d.dft)
		for (auto& b : f) std::memset(b.mapped, 0, d.cells * 2 * sizeof(float));
}

template <class T>
void Vulkan<T>::resetDftAccumulators() {
	auto& d = *impl_;
	for (auto& f : d.dft)
		for (auto& b : f) std::memset(b.mapped, 0, d.cells * 2 * sizeof(float));
}

template <class T>
Grid3<double> Vulkan<T>::collectField(int component) const {
	auto& d = *impl_;
	const Buffer* b[6] = {&d.Ex, &d.Ey, &d.Ez, &d.Hx, &d.Hy, &d.Hz};
	const auto* src = static_cast<const float*>(b[component]->mapped);
	Grid3<double> out(d.Nx, d.Ny, d.Nz);
	for (std::size_t i = 0; i < d.cells; ++i)
		out.data()[i] = static_cast<double>(src[i]);
	return out;
}

template <class T>
void Vulkan<T>::step(int t, bool accumulate_dft) {
	auto& d = *impl_;
	const int Nx = d.Nx, Ny = d.Ny, Nz = d.Nz;

	struct PcUpd {
		int32_t dims[4];
		float delta[4];
	} pcU{{Nx, Ny, Nz, 0}, {d.dx, d.dy, d.dz, 0}};

	const auto [et, mt] = DFTSampleTimes(t, d.dt);

	d.ctx.submitSync([&](VkCommandBuffer cb) {
		d.uh->dispatch(cb, &pcU, sizeof pcU, ceilDiv(Nx - 1, 4),
		               ceilDiv(Ny - 1, 4), ceilDiv(Nz - 1, 4));
		vk::computeBarrier(cb);

		for (std::size_t s = 0; s < d.srcs.size(); ++s) {
			const auto& S = d.srcs[s];
			const double tau = (double(t) - 1.0) * d.dt - S.time_origin;
			struct {
				int32_t a[4], b[4], dd[4];
				float c[4];
			} pc{{S.axis, S.mfixed0, S.r1_lo0, S.r2_lo0},
			     {S.size1, S.size2, S.nfreqs, Nx},
			     {Ny, Nz, 0, 0},
			     {S.scale, float(tau), 0, 0}};
			d.im[s]->dispatch(cb, &pc, sizeof pc, ceilDiv(S.size1, 8),
			                  ceilDiv(S.size2, 8), 1);
		}
		vk::computeBarrier(cb);

		d.ue->dispatch(cb, &pcU, sizeof pcU, ceilDiv(Nx, 4), ceilDiv(Ny, 4),
		               ceilDiv(Nz, 4));
		vk::computeBarrier(cb);

		for (std::size_t s = 0; s < d.srcs.size(); ++s) {
			const auto& S = d.srcs[s];
			const double tau = (double(t) - 0.5) * d.dt - S.time_origin;
			struct {
				int32_t a[4], b[4], dd[4];
				float c[4];
			} pc{{S.axis, S.efixed0, S.r1_lo0, S.r2_lo0},
			     {S.size1, S.size2, S.nfreqs, Nx},
			     {Ny, Nz, 0, 0},
			     {S.scale, float(tau), 0, 0}};
			d.ie[s]->dispatch(cb, &pc, sizeof pc, ceilDiv(S.size1, 8),
			                  ceilDiv(S.size2, 8), 1);
		}
		vk::computeBarrier(cb);

		if (accumulate_dft)
			for (std::size_t f = 0; f < d.freqs.size(); ++f) {
				const auto pe = DFTPhase(d.freqs[f], et);
				const auto ph = DFTPhase(d.freqs[f], mt);
				struct {
					int32_t dims[4];
					float phase[4];
				} pc{{Nx, Ny, Nz, 0},
				     {float(pe.real()), float(pe.imag()), float(ph.real()),
				      float(ph.imag())}};
				d.adft[f]->dispatch(cb, &pc, sizeof pc, ceilDiv(Nx, 4),
				                    ceilDiv(Ny, 4), ceilDiv(Nz, 4));
			}
	});
}

template <class T>
FreqsField Vulkan<T>::collectFreqFields() const {
	auto& d = *impl_;
	FreqsField ff;
	for (std::size_t f = 0; f < d.freqs.size(); ++f) {
		FreqFields g;
		for (int c = 0; c < 3; ++c) {
			g.E[c] = Grid3<cdouble>(d.Nx, d.Ny, d.Nz, cdouble{});
			g.H[c] = Grid3<cdouble>(d.Nx, d.Ny, d.Nz, cdouble{});
			const auto* pe = static_cast<const float*>(d.dft[f][c].mapped);
			const auto* ph = static_cast<const float*>(d.dft[f][c + 3].mapped);
			for (std::size_t i = 0; i < d.cells; ++i) {
				g.E[c].data()[i] = cdouble(pe[2 * i + 0], pe[2 * i + 1]);
				g.H[c].data()[i] = cdouble(ph[2 * i + 0], ph[2 * i + 1]);
			}
		}
		ff.emplace(d.freqs[f], std::move(g));
	}
	return ff;
}

template class Vulkan<float>;

}  // namespace lucuma::julia
