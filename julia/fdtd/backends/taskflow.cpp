// No direct Julia counterpart — mirrors fdtd-lucuma/src/services/backends/cpu_taskflow.cpp.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fdtd/backends/taskflow.hpp"

#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

namespace lucuma::vulkan {

template <class T>
Taskflow<T>::Taskflow() : exec_(std::make_unique<tf::Executor>()) {}

template <class T>
Taskflow<T>::~Taskflow() = default;

template <class T>
void Taskflow<T>::init(const FDTDParams& p) {
	p_ = &p;
	const int Nx = p.whole_region_sizes[0], Ny = p.whole_region_sizes[1],
	          Nz = p.whole_region_sizes[2];
	Ex_ = Grid3<T>(Nx, Ny, Nz);
	Ey_ = Grid3<T>(Nx, Ny, Nz);
	Ez_ = Grid3<T>(Nx, Ny, Nz);
	Hx_ = Grid3<T>(Nx, Ny, Nz);
	Hy_ = Grid3<T>(Nx, Ny, Nz);
	Hz_ = Grid3<T>(Nx, Ny, Nz);
	FillCoeffsWholeGrid<T>(Chh_, Che_, Cee_, Ceh_, p);
	dft_.init(p.freqs, Nx, Ny, Nz);
}

template <class T>
void Taskflow<T>::updateOptRegionCoeffs(const FDTDParams& p) {
	FillCoeffsOptBox<T>(Chh_, Che_, Cee_, Ceh_, p);
}

template <class T>
void Taskflow<T>::reset() {
	Ex_.fill(T(0));
	Ey_.fill(T(0));
	Ez_.fill(T(0));
	Hx_.fill(T(0));
	Hy_.fill(T(0));
	Hz_.fill(T(0));
	dft_.zero();
}

template <class T>
void Taskflow<T>::step(int t, bool accumulate_dft) {
	const FDTDParams& p = *p_;
	const int Nx = p.whole_region_sizes[0], Ny = p.whole_region_sizes[1],
	          Nz = p.whole_region_sizes[2];
	const T dx = T(p.dx), dy = T(p.dy), dz = T(p.dz);

	tf::Taskflow flow;

	auto h = flow.for_each_index(1, Nz, 1, [&](int k) {
		UpdateH<T>(Hx_, Hy_, Hz_, Ex_, Ey_, Ez_, Chh_, Che_, Nx - 1, Ny - 1, k,
		           k, dx, dy, dz);
	});
	auto inj_m = flow.emplace([&] {
		InjectMagneticSources<T>(Hx_, Hy_, Hz_, Che_, p.sources, t, p.dt);
	});
	auto e = flow.for_each_index(2, Nz, 1, [&](int k) {
		UpdateE<T>(Ex_, Ey_, Ez_, Hx_, Hy_, Hz_, Cee_, Ceh_, Nx, Ny, k, k, dx,
		           dy, dz);
	});
	auto inj_e = flow.emplace([&] {
		InjectElectricSources<T>(Ex_, Ey_, Ez_, Ceh_, p.sources, t, p.dt);
	});
	h.precede(inj_m);
	inj_m.precede(e);
	e.precede(inj_e);

	if (accumulate_dft) {
		auto dft = flow.for_each_index(1, Nz + 1, 1, [&](int k) {
			dft_.accumulate(t, p.dt, Ex_, Ey_, Ez_, Hx_, Hy_, Hz_, k, k);
		});
		inj_e.precede(dft);
	}

	exec_->run(flow).wait();
}

template <class T>
void Taskflow<T>::resetDftAccumulators() {
	dft_.zero();
}

template <class T>
FreqsField Taskflow<T>::collectFreqFields() const {
	return dft_.toHost();
}

template <class T>
Grid3<double> Taskflow<T>::collectField(int component) const {
	const Grid3<T>* g[6] = {&Ex_, &Ey_, &Ez_, &Hx_, &Hy_, &Hz_};
	return CopyGridToHost<T>(*g[component]);
}

template class Taskflow<float>;
template class Taskflow<double>;

}  // namespace lucuma::vulkan
