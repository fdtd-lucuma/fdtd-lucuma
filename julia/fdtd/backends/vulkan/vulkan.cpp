// No direct Julia counterpart — mirrors fdtd-lucuma/src/services/backends/vulkan.cpp.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fdtd/backends/vulkan/vulkan.hpp"

#include <array>
#include <cstdint>
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

namespace lucuma::vulkan {

using vk::Buffer;
using vk::ComputePipeline;
using vk::Context;

namespace {
inline uint32_t ceilDiv(int a, int b) {
	return static_cast<uint32_t>((a + b - 1) / b);
}

// Maximum frequencies handled in a single DFT dispatch (must match shader
// MAX_FREQS and the push-constant layout).
static constexpr int kMaxDftFreqsPerBatch = 8;
}  // namespace

template <class T>
struct Vulkan<T>::Impl {
	Context ctx;

	int Nx = 0, Ny = 0, Nz = 0;
	std::size_t cells = 0;
	double dt = 0.0;
	float dx = 0, dy = 0, dz = 0;
	std::vector<double> freqs;

	// All field / coefficient / DFT buffers are DEVICE_LOCAL (VRAM on discrete
	// GPUs).  Source J/M/omega buffers are also device-local; uploads go through
	// a transient staging buffer via ctx.uploadToDevice().
	Buffer Ex, Ey, Ez, Hx, Hy, Hz;
	Buffer Chh, Che, Cee, Ceh;
	Grid3<float> chhCpu, cheCpu, ceeCpu, cehCpu;  // CPU mirror for opt-box refill

	// DFT accumulators: per frequency, 6 components (E1,E2,E3,H1,H2,H3).
	// Buffer layout: dftBuf[f * cells + idx]  (frequency-major, cell-minor).
	// All 6 components share the same frequency×cells stride so we keep them as
	// 6 flat buffers each of size freqs.size() * cells * sizeof(vec2).
	std::array<Buffer, 6> dft;  // E1 E2 E3 H1 H2 H3

	struct Src {
		int axis, efixed0, mfixed0, r1_lo0, r2_lo0, size1, size2, nfreqs;
		float scale;
		double time_origin;
		Buffer Jx, Jy, Jz, Mx, My, Mz, omegas;
	};
	std::vector<Src> srcs;
	Buffer srcStub;  // bound to update_e/h source slots when srcs.size() != 1

	std::unique_ptr<ComputePipeline> uh, ue;
	std::vector<std::unique_ptr<ComputePipeline>> ie, im;  // per source
	std::unique_ptr<ComputePipeline> adft;                 // single multi-freq pipeline

	// Allocate a buffer (preferring DEVICE_LOCAL|HOST_VISIBLE for zero-copy on UMA).
	Buffer mk(std::size_t bytes) { return ctx.createBuffer(bytes); }

	void uploadCoeffsWhole() {
		const std::size_t bytes = cells * sizeof(float);
		if (Chh.mapped) {
			// HOST_VISIBLE (UMA / Metal shared): plain memcpy, no GPU submission.
			std::memcpy(Chh.mapped, chhCpu.data(), bytes);
			std::memcpy(Che.mapped, cheCpu.data(), bytes);
			std::memcpy(Cee.mapped, ceeCpu.data(), bytes);
			std::memcpy(Ceh.mapped, cehCpu.data(), bytes);
		} else {
			// Pure DEVICE_LOCAL: batch all four uploads in ONE submitSync.
			auto stgChh = ctx.createBuffer(bytes, false);
			auto stgChe = ctx.createBuffer(bytes, false);
			auto stgCee = ctx.createBuffer(bytes, false);
			auto stgCeh = ctx.createBuffer(bytes, false);
			std::memcpy(stgChh.mapped, chhCpu.data(), bytes);
			std::memcpy(stgChe.mapped, cheCpu.data(), bytes);
			std::memcpy(stgCee.mapped, ceeCpu.data(), bytes);
			std::memcpy(stgCeh.mapped, cehCpu.data(), bytes);
			ctx.submitSync([&](VkCommandBuffer cb) {
				Context::recordCopy(cb, stgChh.buffer, Chh.buffer, bytes);
				Context::recordCopy(cb, stgChe.buffer, Che.buffer, bytes);
				Context::recordCopy(cb, stgCee.buffer, Cee.buffer, bytes);
				Context::recordCopy(cb, stgCeh.buffer, Ceh.buffer, bytes);
			});
			ctx.destroy(stgChh); ctx.destroy(stgChe);
			ctx.destroy(stgCee); ctx.destroy(stgCeh);
		}
	}

	// Record up to kBatchSteps Yee steps into one reused command buffer,
	// submit + wait once per batch instead of once per step.
	static constexpr int kBatchSteps = 2048;
	VkCommandBuffer batchCb    = VK_NULL_HANDLE;
	VkFence         batchFence = VK_NULL_HANDLE;
	bool recording  = false;
	int  recorded   = 0;

	void beginBatch() {
		if (recording) return;
		if (batchCb == VK_NULL_HANDLE) {
			VkCommandBufferAllocateInfo ai{};
			ai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			ai.commandPool        = ctx.commandPool();
			ai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			ai.commandBufferCount = 1;
			vkAllocateCommandBuffers(ctx.device(), &ai, &batchCb);
			VkFenceCreateInfo fi{};
			fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			vkCreateFence(ctx.device(), &fi, nullptr, &batchFence);
		}
		vkResetCommandBuffer(batchCb, 0);
		VkCommandBufferBeginInfo bi{};
		bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(batchCb, &bi);
		recording = true;
		recorded  = 0;
	}

	void flushBatch() {
		if (!recording) return;
		vkEndCommandBuffer(batchCb);
		VkSubmitInfo si{};
		si.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		si.commandBufferCount = 1;
		si.pCommandBuffers    = &batchCb;
		vkResetFences(ctx.device(), 1, &batchFence);
		vkQueueSubmit(ctx.queue(), 1, &si, batchFence);
		vkWaitForFences(ctx.device(), 1, &batchFence, VK_TRUE, UINT64_MAX);
		recording = false;
	}
};

// ---------------------------------------------------------------------------

template <class T>
Vulkan<T>::Vulkan() : impl_(std::make_unique<Impl>()) {}

template <class T>
Vulkan<T>::~Vulkan() {
	if (!impl_) return;
	auto& d = *impl_;
	d.flushBatch();
	if (d.batchFence) vkDestroyFence(d.ctx.device(), d.batchFence, nullptr);
	for (Buffer* b : {&d.Ex, &d.Ey, &d.Ez, &d.Hx, &d.Hy, &d.Hz,
	                  &d.Chh, &d.Che, &d.Cee, &d.Ceh, &d.srcStub})
		d.ctx.destroy(*b);
	for (auto& b : d.dft) d.ctx.destroy(b);
	for (auto& s : d.srcs)
		for (Buffer* b : {&s.Jx, &s.Jy, &s.Jz, &s.Mx, &s.My, &s.Mz, &s.omegas})
			d.ctx.destroy(*b);
}

template <class T>
void Vulkan<T>::init(const FDTDParams& p) {
	auto& d = *impl_;
	d.Nx    = p.whole_region_sizes[0];
	d.Ny    = p.whole_region_sizes[1];
	d.Nz    = p.whole_region_sizes[2];
	d.cells = std::size_t(d.Nx) * d.Ny * d.Nz;
	d.dt    = p.dt;
	d.dx    = float(p.dx);
	d.dy    = float(p.dy);
	d.dz    = float(p.dz);
	d.freqs = p.freqs;

	const std::size_t fbytes = d.cells * sizeof(float);
	d.Ex = d.mk(fbytes); d.Ey = d.mk(fbytes); d.Ez = d.mk(fbytes);
	d.Hx = d.mk(fbytes); d.Hy = d.mk(fbytes); d.Hz = d.mk(fbytes);
	d.Chh = d.mk(fbytes); d.Che = d.mk(fbytes);
	d.Cee = d.mk(fbytes); d.Ceh = d.mk(fbytes);

	FillCoeffsWholeGrid<float>(d.chhCpu, d.cheCpu, d.ceeCpu, d.cehCpu, p);
	d.uploadCoeffsWhole();

	// DFT buffers: 6 components, each freq-major (nfreqs * cells * sizeof(vec2))
	const std::size_t dftBufBytes = d.freqs.size() * d.cells * 2 * sizeof(float);
	for (auto& b : d.dft) b = d.mk(dftBufBytes);

	for (const auto& s : p.sources) {
		typename Impl::Src S;
		S.axis         = int(s.axis);
		S.efixed0      = int(s.electric_fixed_index) - 1;
		S.mfixed0      = int(s.magnetic_fixed_index) - 1;
		S.r1_lo0       = int(s.r1_lo) - 1;
		S.r2_lo0       = int(s.r2_lo) - 1;
		S.size1        = s.size1;
		S.size2        = s.size2;
		S.nfreqs       = s.nfreqs;
		S.scale        = float(s.J0 * s.inv_delta_normal);
		S.time_origin  = s.time_origin;
		const std::size_t sheet = std::size_t(s.size1) * s.size2 * s.nfreqs;

		// Build a CPU-side float buffer of interleaved (re, im) and upload.
		auto up = [&](const std::vector<std::complex<double>>& src) {
			std::vector<float> tmp(sheet * 2);
			for (std::size_t i = 0; i < sheet; ++i) {
				tmp[2 * i + 0] = float(src[i].real());
				tmp[2 * i + 1] = float(src[i].imag());
			}
			Buffer b = d.mk(sheet * 2 * sizeof(float));
			d.ctx.uploadToDevice(b, tmp.data(), sheet * 2 * sizeof(float));
			return b;
		};
		S.Jx = up(s.Jx); S.Jy = up(s.Jy); S.Jz = up(s.Jz);
		S.Mx = up(s.Mx); S.My = up(s.My); S.Mz = up(s.Mz);

		{
			std::vector<float> ws(s.nfreqs);
			for (int n = 0; n < s.nfreqs; ++n) ws[n] = float(s.omegas[n]);
			S.omegas = d.mk(s.nfreqs * sizeof(float));
			d.ctx.uploadToDevice(S.omegas, ws.data(), s.nfreqs * sizeof(float));
		}
		d.srcs.push_back(std::move(S));
	}

	// ---- pipelines ----
	auto spv = [](const uint32_t* a, std::size_t n) {
		return std::span<const uint32_t>(a, n);
	};

	// update_e/h fold the surface-current injection in when there is exactly one
	// source (the common case) — one dispatch, no extra barrier. Slots 8..11 are
	// bound to that source's J/M/omega buffers, or to a stub otherwise.
	d.srcStub = d.mk(4 * sizeof(float));
	const bool fuse = d.srcs.size() == 1;
	const VkBuffer sbM[4] = {
	    fuse ? d.srcs[0].Mx.buffer : d.srcStub.buffer,
	    fuse ? d.srcs[0].My.buffer : d.srcStub.buffer,
	    fuse ? d.srcs[0].Mz.buffer : d.srcStub.buffer,
	    fuse ? d.srcs[0].omegas.buffer : d.srcStub.buffer};
	const VkBuffer sbJ[4] = {
	    fuse ? d.srcs[0].Jx.buffer : d.srcStub.buffer,
	    fuse ? d.srcs[0].Jy.buffer : d.srcStub.buffer,
	    fuse ? d.srcs[0].Jz.buffer : d.srcStub.buffer,
	    fuse ? d.srcs[0].omegas.buffer : d.srcStub.buffer};

	constexpr uint32_t kUpdPush = 5 * 4 * sizeof(float);  // dims,delta,s0,s1,s2

	d.uh = std::make_unique<ComputePipeline>(
	    d.ctx, spv(update_h_comp_spv, std::size(update_h_comp_spv)), 12, kUpdPush);
	d.uh->bindBuffers(std::array<VkBuffer, 12>{
	    d.Hx.buffer, d.Hy.buffer, d.Hz.buffer, d.Ex.buffer, d.Ey.buffer,
	    d.Ez.buffer, d.Chh.buffer, d.Che.buffer, sbM[0], sbM[1], sbM[2], sbM[3]});

	d.ue = std::make_unique<ComputePipeline>(
	    d.ctx, spv(update_e_comp_spv, std::size(update_e_comp_spv)), 12, kUpdPush);
	d.ue->bindBuffers(std::array<VkBuffer, 12>{
	    d.Ex.buffer, d.Ey.buffer, d.Ez.buffer, d.Hx.buffer, d.Hy.buffer,
	    d.Hz.buffer, d.Cee.buffer, d.Ceh.buffer, sbJ[0], sbJ[1], sbJ[2], sbJ[3]});

	// Inject pipelines — one per source, unchanged workgroup (8×8×1)
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

	// Single multi-frequency DFT pipeline
	// Push-constant layout (must match accum_dft.comp):
	//   ivec4 dims (Nx,Ny,Nz,nfreqs), int firstFreq + 3 pad ints,
	//   vec2 phaseE[8], vec2 phaseH[8]
	//   = 16 + 16 + 64 + 64 = 160 bytes
	constexpr uint32_t kDftPushBytes =
	    sizeof(int32_t) * 4    // dims
	    + sizeof(int32_t) * 4  // firstFreq + 3 pad
	    + sizeof(float) * 2 * kMaxDftFreqsPerBatch   // phaseE
	    + sizeof(float) * 2 * kMaxDftFreqsPerBatch;  // phaseH

	d.adft = std::make_unique<ComputePipeline>(
	    d.ctx, spv(accum_dft_comp_spv, std::size(accum_dft_comp_spv)), 12,
	    kDftPushBytes);
	d.adft->bindBuffers(std::array<VkBuffer, 12>{
	    d.dft[0].buffer, d.dft[1].buffer, d.dft[2].buffer,
	    d.dft[3].buffer, d.dft[4].buffer, d.dft[5].buffer,
	    d.Ex.buffer, d.Ey.buffer, d.Ez.buffer,
	    d.Hx.buffer, d.Hy.buffer, d.Hz.buffer});
}

template <class T>
void Vulkan<T>::updateOptRegionCoeffs(const FDTDParams& p) {
	auto& d = *impl_;
	d.flushBatch();
	FillCoeffsOptBox<float>(d.chhCpu, d.cheCpu, d.ceeCpu, d.cehCpu, p);
	// Re-upload the whole coefficient grid.  The opt-box is typically a small
	// fraction of the domain, but FillCoeffsOptBox touches the full CPU grids
	// and doesn't expose a bounding box, so we upload all four arrays.
	d.uploadCoeffsWhole();
}

template <class T>
void Vulkan<T>::reset() {
	auto& d = *impl_;
	d.flushBatch();
	if (d.Ex.mapped) {
		// HOST_VISIBLE buffers (UMA / Metal shared): memset directly, no GPU work.
		for (Buffer* b : {&d.Ex, &d.Ey, &d.Ez, &d.Hx, &d.Hy, &d.Hz})
			std::memset(b->mapped, 0, static_cast<size_t>(b->size));
		for (auto& b : d.dft) std::memset(b.mapped, 0, static_cast<size_t>(b.size));
	} else {
		// Pure DEVICE_LOCAL: batch ALL fills into a single GPU submission.
		d.ctx.submitSync([&](VkCommandBuffer cb) {
			for (Buffer* b : {&d.Ex, &d.Ey, &d.Ez, &d.Hx, &d.Hy, &d.Hz})
				Context::recordFill(cb, *b, 0);
			for (auto& b : d.dft) Context::recordFill(cb, b, 0);
		});
	}
}

template <class T>
void Vulkan<T>::resetDftAccumulators() {
	auto& d = *impl_;
	d.flushBatch();
	if (d.dft[0].mapped) {
		for (auto& b : d.dft) std::memset(b.mapped, 0, static_cast<size_t>(b.size));
	} else {
		d.ctx.submitSync([&](VkCommandBuffer cb) {
			for (auto& b : d.dft) Context::recordFill(cb, b, 0);
		});
	}
}

template <class T>
Grid3<double> Vulkan<T>::collectField(int component) const {
	auto& d = *impl_;
	d.flushBatch();
	const Buffer* bufs[6] = {&d.Ex, &d.Ey, &d.Ez, &d.Hx, &d.Hy, &d.Hz};
	std::vector<float> tmp(d.cells);
	d.ctx.downloadFromDevice(*bufs[component], tmp.data(),
	                         d.cells * sizeof(float));
	Grid3<double> out(d.Nx, d.Ny, d.Nz);
	for (std::size_t i = 0; i < d.cells; ++i)
		out.data()[i] = static_cast<double>(tmp[i]);
	return out;
}

template <class T>
void Vulkan<T>::step(int t, bool accumulate_dft) {
	auto& d = *impl_;
	const int Nx = d.Nx, Ny = d.Ny, Nz = d.Nz;

	struct PcUpd {
		int32_t dims[4];
		float   delta[4];
		int32_t s0[4];   // src: axis, fixed0, r1_lo0, r2_lo0
		int32_t s1[4];   // src: size1, size2, nfreqs, enable
		float   s2[4];   // src: scale, tau, _, _
	};

	const auto [et, mt] = DFTSampleTimes(t, d.dt);
	const bool fused = d.srcs.size() == 1;

	auto pcFor = [&](int fixed0, double tau) {
		PcUpd p{{Nx, Ny, Nz, 0},
		        {1.0f / d.dx, 1.0f / d.dy, 1.0f / d.dz, 0},
		        {}, {}, {}};
		if (fused) {
			const auto& S = d.srcs[0];
			p.s0[0] = S.axis; p.s0[1] = fixed0;
			p.s0[2] = S.r1_lo0; p.s0[3] = S.r2_lo0;
			p.s1[0] = S.size1; p.s1[1] = S.size2; p.s1[2] = S.nfreqs; p.s1[3] = 1;
			p.s2[0] = S.scale; p.s2[1] = float(tau);
		}
		return p;
	};

	d.beginBatch();
	VkCommandBuffer cb = d.batchCb;

	const int hFixed = fused ? d.srcs[0].mfixed0 : 0;
	const int eFixed = fused ? d.srcs[0].efixed0 : 0;
	const PcUpd pcH = pcFor(hFixed, (double(t) - 1.0) * d.dt -
	                                    (fused ? d.srcs[0].time_origin : 0.0));
	const PcUpd pcE = pcFor(eFixed, (double(t) - 0.5) * d.dt -
	                                    (fused ? d.srcs[0].time_origin : 0.0));

	d.uh->dispatch(cb, &pcH, sizeof pcH,
	               ceilDiv(Nx - 1, 16), ceilDiv(Ny - 1, 16), Nz);
	vk::computeBarrier(cb);

	if (!fused)
		for (std::size_t s = 0; s < d.srcs.size(); ++s) {
			const auto& S = d.srcs[s];
			const double tau = (double(t) - 1.0) * d.dt - S.time_origin;
			struct {
				int32_t a[4], b[4], dd[4];
				float   c[4];
			} pc{{S.axis, S.mfixed0, S.r1_lo0, S.r2_lo0},
			     {S.size1, S.size2, S.nfreqs, Nx},
			     {Ny, Nz, 0, 0},
			     {S.scale, float(tau), 0, 0}};
			d.im[s]->dispatch(cb, &pc, sizeof pc,
			                  ceilDiv(S.size1, 8), ceilDiv(S.size2, 8), 1);
		}
	if (!fused && !d.srcs.empty()) vk::computeBarrier(cb);

	d.ue->dispatch(cb, &pcE, sizeof pcE,
	               ceilDiv(Nx, 16), ceilDiv(Ny, 16), Nz);
	vk::computeBarrier(cb);

	if (!fused)
		for (std::size_t s = 0; s < d.srcs.size(); ++s) {
			const auto& S = d.srcs[s];
			const double tau = (double(t) - 0.5) * d.dt - S.time_origin;
			struct {
				int32_t a[4], b[4], dd[4];
				float   c[4];
			} pc{{S.axis, S.efixed0, S.r1_lo0, S.r2_lo0},
			     {S.size1, S.size2, S.nfreqs, Nx},
			     {Ny, Nz, 0, 0},
			     {S.scale, float(tau), 0, 0}};
			d.ie[s]->dispatch(cb, &pc, sizeof pc,
			                  ceilDiv(S.size1, 8), ceilDiv(S.size2, 8), 1);
		}
	if (!fused && !d.srcs.empty()) vk::computeBarrier(cb);

	// DFT accumulation — single dispatch batching all frequencies
	if (accumulate_dft && !d.freqs.empty()) {
		// Push-constant layout must match accum_dft.comp:
		//   ivec4 dims (Nx,Ny,Nz,nfreqsThisBatch)
		//   int   firstFreq, _pad×3
		//   vec2  phaseE[MAX_FREQS]
		//   vec2  phaseH[MAX_FREQS]
		struct DftPC {
			int32_t dims[4];
			int32_t firstFreq, pad0, pad1, pad2;
			float   phaseE[kMaxDftFreqsPerBatch][2];
			float   phaseH[kMaxDftFreqsPerBatch][2];
		};

		const int nTotal = static_cast<int>(d.freqs.size());
		for (int first = 0; first < nTotal; first += kMaxDftFreqsPerBatch) {
			const int batch = std::min(kMaxDftFreqsPerBatch, nTotal - first);
			DftPC pc{};
			pc.dims[0]   = Nx;
			pc.dims[1]   = Ny;
			pc.dims[2]   = Nz;
			pc.dims[3]   = batch;
			pc.firstFreq = first;
			for (int f = 0; f < batch; ++f) {
				const auto pe = DFTPhase(d.freqs[first + f], et);
				const auto ph = DFTPhase(d.freqs[first + f], mt);
				pc.phaseE[f][0] = float(pe.real());
				pc.phaseE[f][1] = float(pe.imag());
				pc.phaseH[f][0] = float(ph.real());
				pc.phaseH[f][1] = float(ph.imag());
			}
			d.adft->dispatch(cb, &pc, sizeof pc,
			                 ceilDiv(Nx, 8), ceilDiv(Ny, 8), ceilDiv(Nz, 4));
		}
		vk::computeBarrier(cb);
	}

	if (++d.recorded >= Impl::kBatchSteps) d.flushBatch();
}

template <class T>
FreqsField Vulkan<T>::collectFreqFields() const {
	auto& d = *impl_;
	d.flushBatch();

	const std::size_t nf      = d.freqs.size();
	const std::size_t cells   = d.cells;
	const std::size_t bufBytes = nf * cells * 2 * sizeof(float);

	// Download all 6 DFT buffers.  Each is freq-major: buf[f * cells + idx].
	// When buffers are HOST_VISIBLE we read directly; otherwise batch all 6
	// downloads into a single GPU submission with temporary staging buffers.
	std::vector<std::array<std::vector<float>, 6>> raw(nf);
	if (d.dft[0].mapped) {
		// HOST_VISIBLE path: memcpy directly from mapped memory.
		for (int c = 0; c < 6; ++c) {
			const float* src = static_cast<const float*>(d.dft[c].mapped);
			for (std::size_t f = 0; f < nf; ++f) {
				raw[f][c].resize(cells * 2);
				std::memcpy(raw[f][c].data(), src + f * cells * 2,
				            cells * 2 * sizeof(float));
			}
		}
	} else {
		// Pure DEVICE_LOCAL: allocate 6 staging buffers, batch the copies.
		std::array<Buffer, 6> stg;
		for (int c = 0; c < 6; ++c)
			stg[c] = d.ctx.createBuffer(bufBytes, /*preferDeviceLocal=*/false);
		d.ctx.submitSync([&](VkCommandBuffer cb) {
			for (int c = 0; c < 6; ++c)
				Context::recordCopy(cb, d.dft[c].buffer, stg[c].buffer, bufBytes);
		});
		for (int c = 0; c < 6; ++c) {
			const float* src = static_cast<const float*>(stg[c].mapped);
			for (std::size_t f = 0; f < nf; ++f) {
				raw[f][c].resize(cells * 2);
				std::memcpy(raw[f][c].data(), src + f * cells * 2,
				            cells * 2 * sizeof(float));
			}
			d.ctx.destroy(stg[c]);
		}
	}

	FreqsField ff;
	for (std::size_t f = 0; f < nf; ++f) {
		FreqFields g;
		for (int c = 0; c < 3; ++c) {
			g.E[c] = Grid3<cdouble>(d.Nx, d.Ny, d.Nz, cdouble{});
			g.H[c] = Grid3<cdouble>(d.Nx, d.Ny, d.Nz, cdouble{});
			const float* pe = raw[f][c].data();
			const float* ph = raw[f][c + 3].data();
			for (std::size_t i = 0; i < cells; ++i) {
				g.E[c].data()[i] = cdouble(pe[2 * i + 0], pe[2 * i + 1]);
				g.H[c].data()[i] = cdouble(ph[2 * i + 0], ph[2 * i + 1]);
			}
		}
		ff.emplace(d.freqs[f], std::move(g));
	}
	return ff;
}

template class Vulkan<float>;

}  // namespace lucuma::vulkan
