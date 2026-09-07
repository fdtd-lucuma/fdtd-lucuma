// No direct Julia counterpart — mirrors fdtd-lucuma/src/services/backends/sequential.cpp.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fdtd/backends/sequential.hpp"

namespace lucuma::vulkan {

template <class T>
void Sequential<T>::init(const FDTDParams& p) {
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
void Sequential<T>::updateOptRegionCoeffs(const FDTDParams& p) {
	FillCoeffsOptBox<T>(Chh_, Che_, Cee_, Ceh_, p);
}

template <class T>
void Sequential<T>::reset() {
	Ex_.fill(T(0));
	Ey_.fill(T(0));
	Ez_.fill(T(0));
	Hx_.fill(T(0));
	Hy_.fill(T(0));
	Hz_.fill(T(0));
	dft_.zero();
}

template <class T>
void Sequential<T>::step(int t, bool accumulate_dft) {
	const FDTDParams& p = *p_;
	const int Nx = p.whole_region_sizes[0], Ny = p.whole_region_sizes[1],
	          Nz = p.whole_region_sizes[2];
	const T dx = T(p.dx), dy = T(p.dy), dz = T(p.dz);

	UpdateH<T>(Hx_, Hy_, Hz_, Ex_, Ey_, Ez_, Chh_, Che_, Nx - 1, Ny - 1, 1,
	           Nz - 1, dx, dy, dz);
	InjectMagneticSources<T>(Hx_, Hy_, Hz_, Che_, p.sources, t, p.dt);

	UpdateE<T>(Ex_, Ey_, Ez_, Hx_, Hy_, Hz_, Cee_, Ceh_, Nx, Ny, 2, Nz, dx, dy,
	           dz);
	InjectElectricSources<T>(Ex_, Ey_, Ez_, Ceh_, p.sources, t, p.dt);

	if (accumulate_dft)
		dft_.accumulate(t, p.dt, Ex_, Ey_, Ez_, Hx_, Hy_, Hz_);
}

template <class T>
void Sequential<T>::resetDftAccumulators() {
	dft_.zero();
}

template <class T>
FreqsField Sequential<T>::collectFreqFields() const {
	return dft_.toHost();
}

template <class T>
Grid3<double> Sequential<T>::collectField(int component) const {
	const Grid3<T>* g[6] = {&Ex_, &Ey_, &Ez_, &Hx_, &Hy_, &Hz_};
	return CopyGridToHost<T>(*g[component]);
}

template class Sequential<float>;
template class Sequential<double>;

}  // namespace lucuma::vulkan
