// == auxiliaries/modes.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auxiliaries/modes.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

#include <Eigen/Eigenvalues>
#include <Eigen/SparseCore>
#include <Spectra/GenEigsSolver.h>
#include <Spectra/MatOp/SparseGenMatProd.h>

namespace lucuma::vulkan {

using Eigen::MatrixXcd;
using Eigen::MatrixXd;
using Eigen::VectorXcd;

// ---------------------------------------------------------------------------
// grid helpers
// ---------------------------------------------------------------------------

IdxRange ValueRangeToIndexRange(double b0, double b1, double step) {
	return {julia_round(b0 / step) + 1, julia_round(b1 / step)};
}

long CenterIndex(const IdxRange& r) {
	return julia_round((static_cast<double>(r.first()) + r.last()) / 2.0);
}

IdxRange ExpandRange(const IdxRange& r, long pad, long N) {
	return {std::max(1L, r.first() - pad), std::min(N, r.last() + pad)};
}

Axis DirectionAxis(const std::string& d) {
	if (d == "+x" || d == "-x") return Axis::X;
	if (d == "+y" || d == "-y") return Axis::Y;
	if (d == "+z" || d == "-z") return Axis::Z;
	throw std::runtime_error("Unknown direction: " + d);
}

int DirectionPolarity(const std::string& d) {
	if (!d.empty() && d.front() == '+') return +1;
	if (!d.empty() && d.front() == '-') return -1;
	throw std::runtime_error("Unknown direction polarity: " + d);
}

std::pair<double, double> AxisDeltas(const Params& params, Axis axis) {
	const double dx = TomlNum(params, "domain", "dx");
	const double dy = TomlNum(params, "domain", "dy");
	const double dz = TomlNum(params, "domain", "dz");
	switch (axis) {
		case Axis::X: return {dy, dz};
		case Axis::Y: return {dx, dz};
		default: return {dx, dy};  // Axis::Z
	}
}

// ---------------------------------------------------------------------------
// epsilon grid
// ---------------------------------------------------------------------------

static IdxRange coord_range(const Params& device, const std::string& name,
                            const char* coord, double step) {
	auto arr = device[name][coord].as_array();
	return ValueRangeToIndexRange((*arr)[0].value<double>().value(),
	                              (*arr)[1].value<double>().value(), step);
}

Grid3<double> BuildRelativePermittivityGrid(
    const Params& params, const Params& device,
    const std::vector<std::string>& waveguides_names,
    const std::vector<double>& p_flat, int nxP, int nyP, int nzP,
    const std::string& opt_name) {

	const double dx = TomlNum(params, "domain", "dx");
	const double dy = TomlNum(params, "domain", "dy");
	const double dz = TomlNum(params, "domain", "dz");

	const long Nx = julia_round(TomlNum(params, "domain", "Lx") / dx);
	const long Ny = julia_round(TomlNum(params, "domain", "Ly") / dy);
	const long Nz = julia_round(TomlNum(params, "domain", "Lz") / dz);

	const double eps_bg = TomlNum(params, "material", "eps_bg");
	const double eps_fg = TomlNum(params, "material", "eps_fg");

	Grid3<double> g(static_cast<int>(Nx), static_cast<int>(Ny),
	                static_cast<int>(Nz), eps_bg);

	for (const auto& name : waveguides_names) {
		const IdxRange xr = coord_range(device, name, "x", dx);
		const IdxRange yr = coord_range(device, name, "y", dy);
		const IdxRange zr = coord_range(device, name, "z", dz);
		for (long k = zr.lo; k <= zr.hi; ++k)
			for (long j = yr.lo; j <= yr.hi; ++j)
				for (long i = xr.lo; i <= xr.hi; ++i)
					g(static_cast<int>(i), static_cast<int>(j),
					  static_cast<int>(k)) = eps_fg;
	}

	if (!p_flat.empty()) {
		const IdxRange xr = coord_range(device, opt_name, "x", dx);
		const IdxRange yr = coord_range(device, opt_name, "y", dy);
		const IdxRange zr = coord_range(device, opt_name, "z", dz);
		for (int c = 1; c <= nzP; ++c)
			for (int b = 1; b <= nyP; ++b)
				for (int a = 1; a <= nxP; ++a) {
					const double P = p_flat[PIndex(a - 1, b - 1, c - 1, nxP, nyP)];
					g(static_cast<int>(xr.lo) + a - 1,
					  static_cast<int>(yr.lo) + b - 1,
					  static_cast<int>(zr.lo) + c - 1) =
					    eps_bg + P * (eps_fg - eps_bg);
				}
	}

	return g;
}

// ---------------------------------------------------------------------------
// modal cross section
// ---------------------------------------------------------------------------

PortSection ExtractPortSection(const Grid3<double>& g, const BBox& bbox,
                               const Params& params, long transverse_pad,
                               const std::string& direction_override) {
	const long Nx = g.nx(), Ny = g.ny(), Nz = g.nz();

	const IdxRange xr_box{bbox.r[0][0], bbox.r[0][1]};
	const IdxRange yr_box{bbox.r[1][0], bbox.r[1][1]};
	const IdxRange zr_box{bbox.r[2][0], bbox.r[2][1]};

	const std::string dir = !direction_override.empty()
	                            ? direction_override
	                            : bbox.direction.value_or(std::string{});
	const Axis axis = DirectionAxis(dir);
	const int polarity = DirectionPolarity(dir);
	const auto [delta1, delta2] = AxisDeltas(params, axis);

	const double dx = TomlNum(params, "domain", "dx");
	const double dy = TomlNum(params, "domain", "dy");
	const double dz = TomlNum(params, "domain", "dz");

	PortSection sec;
	sec.axis = axis;
	sec.polarity = polarity;
	sec.delta1 = delta1;
	sec.delta2 = delta2;

	if (axis == Axis::Z) {
		const IdxRange xr = ExpandRange(xr_box, transverse_pad, Nx);
		const IdxRange yr = ExpandRange(yr_box, transverse_pad, Ny);
		const long k = CenterIndex(zr_box);
		Mat2<double> e(static_cast<int>(xr.size()), static_cast<int>(yr.size()));
		for (int b = 1; b <= yr.size(); ++b)
			for (int a = 1; a <= xr.size(); ++a)
				e(a, b) = g(static_cast<int>(xr.lo) + a - 1,
				            static_cast<int>(yr.lo) + b - 1,
				            static_cast<int>(k));
		sec.epsr = std::move(e);
		sec.fixed_index = k;
		sec.range1 = xr;
		sec.range2 = yr;
		sec.delta_normal = dz;
	} else if (axis == Axis::X) {
		const IdxRange yr = ExpandRange(yr_box, transverse_pad, Ny);
		const IdxRange zr = ExpandRange(zr_box, transverse_pad, Nz);
		const long i = CenterIndex(xr_box);
		Mat2<double> e(static_cast<int>(yr.size()), static_cast<int>(zr.size()));
		for (int b = 1; b <= zr.size(); ++b)
			for (int a = 1; a <= yr.size(); ++a)
				e(a, b) = g(static_cast<int>(i),
				            static_cast<int>(yr.lo) + a - 1,
				            static_cast<int>(zr.lo) + b - 1);
		sec.epsr = std::move(e);
		sec.fixed_index = i;
		sec.range1 = yr;
		sec.range2 = zr;
		sec.delta_normal = dx;
	} else {  // Axis::Y
		const IdxRange xr = ExpandRange(xr_box, transverse_pad, Nx);
		const IdxRange zr = ExpandRange(zr_box, transverse_pad, Nz);
		const long j = CenterIndex(yr_box);
		Mat2<double> e(static_cast<int>(xr.size()), static_cast<int>(zr.size()));
		for (int b = 1; b <= zr.size(); ++b)
			for (int a = 1; a <= xr.size(); ++a)
				e(a, b) = g(static_cast<int>(xr.lo) + a - 1,
				            static_cast<int>(j),
				            static_cast<int>(zr.lo) + b - 1);
		sec.epsr = std::move(e);
		sec.fixed_index = j;
		sec.range1 = xr;
		sec.range2 = zr;
		sec.delta_normal = dy;
	}

	return sec;
}

// ===========================================================================
// full-vector mode solver
// ===========================================================================

namespace {

// The waveguide operator has real entries for a lossless (real epsr) guide, so
// A is assembled with real scalars and passed to Spectra's real non-symmetric
// Arnoldi solver -- the direct analogue of Julia's Arpack.eigs(A; which=:LR).
// Julia wraps the same data in ComplexF64 only to select ARPACK's complex path.
using SpMat = Eigen::SparseMatrix<double>;
using SpMatC = Eigen::SparseMatrix<cdouble>;
using Trip = Eigen::Triplet<double>;
constexpr cdouble I1i{0.0, 1.0};

SpMat d1_forward(int N, double delta) {
	std::vector<Trip> t;
	for (int i = 0; i < N; ++i) {
		if (i < N - 1) {
			t.emplace_back(i, i + 1, 1.0 / delta);
			t.emplace_back(i, i, -1.0 / delta);
		} else {
			t.emplace_back(i, i, -1.0 / delta);
		}
	}
	SpMat M(N, N);
	M.setFromTriplets(t.begin(), t.end());
	return M;
}

SpMat d1_backward(int N, double delta) {
	std::vector<Trip> t;
	for (int i = 0; i < N; ++i) {
		if (i > 0) {
			t.emplace_back(i, i, 1.0 / delta);
			t.emplace_back(i, i - 1, -1.0 / delta);
		} else {
			t.emplace_back(i, i, 1.0 / delta);
		}
	}
	SpMat M(N, N);
	M.setFromTriplets(t.begin(), t.end());
	return M;
}

SpMat speye(int n) {
	SpMat I(n, n);
	I.setIdentity();
	return I;
}

// Julia kron(A,B): [(ia*Br+ib), (ja*Bc+jb)] = A(ia,ja)*B(ib,jb)
SpMat kron(const SpMat& A, const SpMat& B) {
	std::vector<Trip> t;
	t.reserve(static_cast<std::size_t>(A.nonZeros()) * B.nonZeros());
	for (int ka = 0; ka < A.outerSize(); ++ka)
		for (SpMat::InnerIterator ia(A, ka); ia; ++ia)
			for (int kb = 0; kb < B.outerSize(); ++kb)
				for (SpMat::InnerIterator ib(B, kb); ib; ++ib)
					t.emplace_back(
					    static_cast<int>(ia.row() * B.rows() + ib.row()),
					    static_cast<int>(ia.col() * B.cols() + ib.col()),
					    ia.value() * ib.value());
	SpMat R(A.rows() * B.rows(), A.cols() * B.cols());
	R.setFromTriplets(t.begin(), t.end());
	return R;
}

SpMat vstack(const SpMat& A, const SpMat& B) {  // [A; B], equal cols
	std::vector<Trip> t;
	for (int k = 0; k < A.outerSize(); ++k)
		for (SpMat::InnerIterator it(A, k); it; ++it)
			t.emplace_back(static_cast<int>(it.row()), static_cast<int>(it.col()),
			               it.value());
	for (int k = 0; k < B.outerSize(); ++k)
		for (SpMat::InnerIterator it(B, k); it; ++it)
			t.emplace_back(static_cast<int>(it.row() + A.rows()),
			               static_cast<int>(it.col()), it.value());
	SpMat R(A.rows() + B.rows(), A.cols());
	R.setFromTriplets(t.begin(), t.end());
	return R;
}

SpMat hstack(const SpMat& A, const SpMat& B) {  // [A B], equal rows
	std::vector<Trip> t;
	for (int k = 0; k < A.outerSize(); ++k)
		for (SpMat::InnerIterator it(A, k); it; ++it)
			t.emplace_back(static_cast<int>(it.row()), static_cast<int>(it.col()),
			               it.value());
	for (int k = 0; k < B.outerSize(); ++k)
		for (SpMat::InnerIterator it(B, k); it; ++it)
			t.emplace_back(static_cast<int>(it.row()),
			               static_cast<int>(it.col() + A.cols()), it.value());
	SpMat R(A.rows(), A.cols() + B.cols());
	R.setFromTriplets(t.begin(), t.end());
	return R;
}

SpMat spdiag(const Eigen::VectorXd& v) {
	SpMat D(v.size(), v.size());
	D.reserve(Eigen::VectorXi::Constant(v.size(), 1));
	for (int i = 0; i < v.size(); ++i) D.insert(i, i) = v[i];
	D.makeCompressed();
	return D;
}

struct Derivs {
	SpMatC D1f, D2f, D1b, D2b;  // cast to complex for the field reconstruction
};

// == BuildVectorWaveguideOperator with mu == 1 (never called otherwise).
struct WgOperator {
	SpMat A;  // real 2N x 2N
	Derivs d;
};

WgOperator build_vector_wg_operator(double k0, double d1, double d2,
                                    const MatrixXd& epsr) {
	const int N1 = static_cast<int>(epsr.rows());
	const int N2 = static_cast<int>(epsr.cols());
	const int N = N1 * N2;

	const SpMat D1f_1d = d1_forward(N1, d1);
	const SpMat D1b_1d = d1_backward(N1, d1);
	const SpMat D2f_1d = d1_forward(N2, d2);
	const SpMat D2b_1d = d1_backward(N2, d2);
	const SpMat I1 = speye(N1);
	const SpMat I2 = speye(N2);

	const SpMat D1f = kron(I2, D1f_1d);
	const SpMat D2f = kron(D2f_1d, I1);
	const SpMat D1b = kron(I2, D1b_1d);
	const SpMat D2b = kron(D2b_1d, I1);

	Eigen::VectorXd e(N);
	for (int idx = 0; idx < N; ++idx) e[idx] = epsr.data()[idx];  // vec(epsr)

	Eigen::VectorXd e2e1(2 * N);
	e2e1.head(N) = e;  // vcat(e2, e1), e1 == e2 == e3 == e
	e2e1.tail(N) = e;

	const SpMat E21 = spdiag(e2e1);
	const SpMat Ez_inv = spdiag(e.cwiseInverse());
	const SpMat M12 = speye(2 * N);
	const SpMat Mz_inv = speye(N);

	const SpMat C_forward = vstack(SpMat(-D2f), D1f);   // 2N x N
	const SpMat C_backward = hstack(SpMat(-D2b), D1b);  // N x 2N
	const SpMat G_backward = vstack(D1b, D2b);          // 2N x N
	const SpMat G_forward = hstack(D1f, D2f);           // N x 2N

	SpMat A = (k0 * k0) * (E21 * M12);
	A += E21 * C_forward * Ez_inv * C_backward;
	A += G_backward * Mz_inv * G_forward * M12;
	A.makeCompressed();

	return {A,
	        {D1f.cast<cdouble>(), D2f.cast<cdouble>(), D1b.cast<cdouble>(),
	         D2b.cast<cdouble>()}};
}

struct LocalGrid {
	MatrixXd epsr;
	double d1, d2;
};

// == LocalModeGrid
LocalGrid local_mode_grid(const PortSection& s) {
	MatrixXd E(s.epsr.rows(), s.epsr.cols());
	for (int j = 1; j <= s.epsr.cols(); ++j)
		for (int i = 1; i <= s.epsr.rows(); ++i) E(i - 1, j - 1) = s.epsr(i, j);
	if (s.axis == Axis::Y)
		return {E.transpose(), s.delta2, s.delta1};
	return {E, s.delta1, s.delta2};
}

struct SixFields {
	MatrixXcd Ex, Ey, Ez, Hx, Hy, Hz;
};

// == GlobalComponentsFromLocal
SixFields global_from_local(Axis axis, const MatrixXcd& E1, const MatrixXcd& E2,
                            const MatrixXcd& E3, const MatrixXcd& H1,
                            const MatrixXcd& H2, const MatrixXcd& H3) {
	if (axis == Axis::Z) return {E1, E2, E3, H1, H2, H3};
	if (axis == Axis::X) return {E3, E1, E2, H3, H1, H2};
	// Y: solved in the (z,x,y) local basis -> map back to stored (x,z) order.
	return {E2.transpose(), E3.transpose(), E1.transpose(),
	        H2.transpose(), H3.transpose(), H1.transpose()};
}

// sum(A .* conj(B))
cdouble sum_prod_conj(const MatrixXcd& A, const MatrixXcd& B) {
	return (A.array() * B.array().conjugate()).sum();
}

// == VectorModePower
double vector_mode_power(Axis axis, int polarity, const SixFields& f, double d1,
                         double d2) {
	cdouble s;
	if (axis == Axis::Z)
		s = sum_prod_conj(f.Ex, f.Hy) - sum_prod_conj(f.Ey, f.Hx);
	else if (axis == Axis::X)
		s = sum_prod_conj(f.Ey, f.Hz) - sum_prod_conj(f.Ez, f.Hy);
	else
		s = sum_prod_conj(f.Ez, f.Hx) - sum_prod_conj(f.Ex, f.Hz);
	return 0.5 * std::real(s * (d1 * d2)) * polarity;
}

// == NormalizeModePower (in place)
double normalize_mode_power(Axis axis, int polarity, SixFields& f, double d1,
                            double d2) {
	const double P = vector_mode_power(axis, polarity, f, d1, d2);
	const double scale = 1.0 / std::sqrt(P);
	f.Ex *= scale; f.Ey *= scale; f.Ez *= scale;
	f.Hx *= scale; f.Hy *= scale; f.Hz *= scale;
	return P;
}

cdouble signed_sqrt(cdouble v) {
	cdouble b = std::sqrt(v);
	if (b.real() < 0.0) b = -b;
	return b;
}

}  // namespace

std::vector<VectorModeProfile> SolveVectorModes(const PortSection& section,
                                               double omega, int nmodes,
                                               double c0, double eps0) {
	const LocalGrid lg = local_mode_grid(section);
	const int N1 = static_cast<int>(lg.epsr.rows());
	const int N2 = static_cast<int>(lg.epsr.cols());
	const int N = N1 * N2;
	const double k0 = omega / c0;

	const WgOperator op = build_vector_wg_operator(k0, lg.d1, lg.d2, lg.epsr);

	// Arpack.eigs(A; nev=nmodes, which=:LR): nmodes eigenpairs of largest real
	// part via Arnoldi. ncv follows the ARPACK/Arpack.jl default.
	const int n = static_cast<int>(op.A.rows());
	const int ncv = std::min(n, std::max(2 * nmodes + 1, 20));
	Spectra::SparseGenMatProd<double> matop(op.A);
	Spectra::GenEigsSolver<Spectra::SparseGenMatProd<double>> es(matop, nmodes,
	                                                             ncv);
	es.init();
	es.compute(Spectra::SortRule::LargestReal);
	if (es.info() != Spectra::CompInfo::Successful)
		throw std::runtime_error("mode Arnoldi iteration did not converge");
	const VectorXcd vals = es.eigenvalues();    // length nmodes, LR-sorted
	const MatrixXcd vecs = es.eigenvectors();   // n x nmodes

	// reorder those nmodes by descending real(neff), neff = signed_sqrt(val)/k0.
	std::vector<int> idx(nmodes);
	std::iota(idx.begin(), idx.end(), 0);
	std::stable_sort(idx.begin(), idx.end(), [&](int a, int b) {
		return signed_sqrt(vals[a]).real() > signed_sqrt(vals[b]).real();
	});

	VectorXcd e_local(N);
	for (int i = 0; i < N; ++i) e_local[i] = cdouble(lg.epsr.data()[i], 0.0);

	std::vector<VectorModeProfile> modes(nmodes);
	for (int mi = 0; mi < nmodes; ++mi) {
		const int chosen = idx[mi];
		const cdouble beta = signed_sqrt(vals[chosen]);
		const cdouble eigenvalue = vals[chosen];
		const cdouble neff = beta / k0;

		const VectorXcd h = vecs.col(chosen);
		const VectorXcd h1v = h.head(N);
		const VectorXcd h2v = h.tail(N);

		// H3 = (D1f*vec(H1) + D2f*vec(H2)) / (i*beta)   (mu == 1)
		VectorXcd h3v = op.d.D1f * h1v + op.d.D2f * h2v;
		h3v /= (I1i * beta);

		// E = curl(H) / (i*omega*eps0*eps)
		const VectorXcd denom = (I1i * omega * eps0) * e_local;
		const VectorXcd e1v =
		    (op.d.D2b * h3v + (I1i * beta) * h2v).cwiseQuotient(denom);
		const VectorXcd e2v =
		    (-(I1i * beta) * h1v - op.d.D1b * h3v).cwiseQuotient(denom);
		const VectorXcd e3v =
		    (op.d.D1b * h2v - op.d.D2b * h1v).cwiseQuotient(denom);

		auto as_mat = [&](const VectorXcd& v) {
			return MatrixXcd(Eigen::Map<const MatrixXcd>(v.data(), N1, N2));
		};
		SixFields f = global_from_local(section.axis, as_mat(e1v), as_mat(e2v),
		                                as_mat(e3v), as_mat(h1v), as_mat(h2v),
		                                as_mat(h3v));

		if (section.polarity < 0) {
			f.Ex = f.Ex.conjugate();
			f.Ey = f.Ey.conjugate();
			f.Ez = f.Ez.conjugate();
			f.Hx = -f.Hx.conjugate();
			f.Hy = -f.Hy.conjugate();
			f.Hz = -f.Hz.conjugate();
		}

		const cdouble hphase = std::exp(-I1i * static_cast<double>(section.polarity) *
		                                beta * (section.delta_normal / 2.0));
		f.Hx *= hphase; f.Hy *= hphase; f.Hz *= hphase;

		normalize_mode_power(section.axis, section.polarity, f, section.delta1,
		                     section.delta2);

		modes[mi] = VectorModeProfile{f.Ex, f.Ey, f.Ez, f.Hx, f.Hy, f.Hz,
		                              neff,  beta, eigenvalue,
		                              omega, section.axis, section.polarity};
	}
	return modes;
}

VectorModeProfile SolveVectorMode(const PortSection& section, double omega,
                                  int nmodes, int mode_index, double c0,
                                  double eps0) {
	return SolveVectorModes(section, omega, nmodes, c0, eps0)[mode_index - 1];
}

// ===========================================================================
// tracked mode family + barycentric interpolation
// ===========================================================================

std::vector<double> ChebyshevFrequencySamples(double fmin, double fmax,
                                              int count) {
	const double center = 0.5 * (fmin + fmax);
	const double half = 0.5 * (fmax - fmin);
	std::vector<double> nodes(count);
	for (int i = 0; i < count; ++i)
		nodes[i] = center +
		           half * std::cos(M_PI * (static_cast<double>(i) / (count - 1)));
	std::sort(nodes.begin(), nodes.end());
	return nodes;
}

namespace {

VectorModeProfile scale_mode(const VectorModeProfile& m, cdouble s) {
	VectorModeProfile r = m;
	r.Ex *= s; r.Ey *= s; r.Ez *= s;
	r.Hx *= s; r.Hy *= s; r.Hz *= s;
	return r;
}

// == VectorModeElectricCorrelation ; dot(x,y) == sum(conj(x).*y)
cdouble electric_correlation(const VectorModeProfile& a,
                             const VectorModeProfile& b) {
	const cdouble num = sum_prod_conj(a.Ex, b.Ex) + sum_prod_conj(a.Ey, b.Ey) +
	                    sum_prod_conj(a.Ez, b.Ez);
	const double an =
	    a.Ex.squaredNorm() + a.Ey.squaredNorm() + a.Ez.squaredNorm();
	const double bn =
	    b.Ex.squaredNorm() + b.Ey.squaredNorm() + b.Ez.squaredNorm();
	const double den = std::sqrt(an * bn);
	return num / den;
}

double max_abs(const MatrixXcd& m) {
	double v = 0.0;
	for (int i = 0; i < m.size(); ++i) v = std::max(v, std::abs(m.data()[i]));
	return v;
}

// == CanonicalizeVectorModePhase
VectorModeProfile canonicalize_phase(const VectorModeProfile& m) {
	const MatrixXcd* fields[3] = {&m.Ex, &m.Ey, &m.Ez};
	int comp = 0;
	double best = -1.0;
	for (int c = 0; c < 3; ++c) {
		const double v = max_abs(*fields[c]);
		if (v > best) { best = v; comp = c; }
	}
	const MatrixXcd& field = *fields[comp];
	cdouble value{};
	double bv = -1.0;
	for (int i = 0; i < field.size(); ++i)
		if (std::abs(field.data()[i]) > bv) {
			bv = std::abs(field.data()[i]);
			value = field.data()[i];
		}
	return scale_mode(m, std::conj(value / std::abs(value)));
}

// == AlignVectorModePhase
VectorModeProfile align_phase(const VectorModeProfile& ref,
                              const VectorModeProfile& cand) {
	const cdouble corr = electric_correlation(ref, cand);
	return scale_mode(cand, std::conj(corr / std::abs(corr)));
}

std::vector<double> barycentric_weights(const std::vector<double>& nodes) {
	const int n = static_cast<int>(nodes.size());
	std::vector<double> w(n, 1.0);
	for (int j = 0; j < n; ++j)
		for (int k = 0; k < n; ++k) {
			if (j == k) continue;
			w[j] /= (nodes[j] - nodes[k]);
		}
	return w;
}

template <class T>
T barycentric_interpolate(const std::vector<double>& nodes,
                          const std::vector<T>& values, double target) {
	for (std::size_t i = 0; i < nodes.size(); ++i) {
		const double tol =
		    std::numeric_limits<double>::epsilon() * std::max(std::abs(nodes[i]), 1.0);
		if (std::abs(target - nodes[i]) <= tol) return values[i];
	}
	const std::vector<double> w = barycentric_weights(nodes);
	std::vector<double> coef(nodes.size());
	double denom = 0.0;
	for (std::size_t i = 0; i < nodes.size(); ++i) {
		coef[i] = w[i] / (target - nodes[i]);
		denom += coef[i];
	}
	T result = coef[0] * values[0];
	for (std::size_t i = 1; i < values.size(); ++i) result += coef[i] * values[i];
	return result / denom;
}

}  // namespace

TrackedModeFamily BuildTrackedModeFamily(const PortSection& section,
                                         const std::vector<double>& frequencies,
                                         int nmodes, int initial_mode_index) {
	std::vector<double> freqs = frequencies;
	std::sort(freqs.begin(), freqs.end());

	TrackedModeFamily fam;
	fam.section = section;
	fam.frequencies = freqs;
	fam.modes.resize(freqs.size());

	const auto first = SolveVectorModes(section, 2.0 * M_PI * freqs[0], nmodes);
	fam.modes[0] = canonicalize_phase(first[initial_mode_index - 1]);

	for (std::size_t index = 1; index < freqs.size(); ++index) {
		const auto cands =
		    SolveVectorModes(section, 2.0 * M_PI * freqs[index], nmodes);
		int best = 0;
		double best_sim = -1.0;
		for (std::size_t c = 0; c < cands.size(); ++c) {
			const double sim =
			    std::abs(electric_correlation(fam.modes[index - 1], cands[c]));
			if (sim > best_sim) { best_sim = sim; best = static_cast<int>(c); }
		}
		fam.modes[index] = align_phase(fam.modes[index - 1], cands[best]);
	}
	return fam;
}

VectorModeProfile InterpolateTrackedMode(const TrackedModeFamily& family,
                                         double frequency) {
	const double fmin = family.frequencies.front();
	const double fmax = family.frequencies.back();

	const double center = 0.5 * (fmin + fmax);
	const double half = 0.5 * (fmax - fmin);
	std::vector<double> nodes(family.frequencies.size());
	for (std::size_t i = 0; i < nodes.size(); ++i)
		nodes[i] = (family.frequencies[i] - center) / half;
	const double target = (frequency - center) / half;

	auto interp_field = [&](MatrixXcd VectorModeProfile::*member) {
		std::vector<MatrixXcd> vals;
		for (const auto& m : family.modes) vals.push_back(m.*member);
		return barycentric_interpolate(nodes, vals, target);
	};

	SixFields f;
	f.Ex = interp_field(&VectorModeProfile::Ex);
	f.Ey = interp_field(&VectorModeProfile::Ey);
	f.Ez = interp_field(&VectorModeProfile::Ez);
	f.Hx = interp_field(&VectorModeProfile::Hx);
	f.Hy = interp_field(&VectorModeProfile::Hy);
	f.Hz = interp_field(&VectorModeProfile::Hz);

	std::vector<cdouble> neffs;
	for (const auto& m : family.modes) neffs.push_back(m.neff);
	const cdouble neff = barycentric_interpolate(nodes, neffs, target);
	const cdouble beta = (2.0 * M_PI * frequency / kC0) * neff;

	normalize_mode_power(family.section.axis, family.section.polarity, f,
	                     family.section.delta1, family.section.delta2);

	return VectorModeProfile{f.Ex,          f.Ey,
	                         f.Ez,          f.Hx,
	                         f.Hy,          f.Hz,
	                         neff,          beta,
	                         beta * beta,   2.0 * M_PI * frequency,
	                         family.section.axis, family.section.polarity};
}

// ===========================================================================
// mode monitors + modal overlap
// ===========================================================================

ModeMonitor MakeModeMonitor(const PortSection& section,
                            std::map<double, VectorModeProfile> modes_by_freq,
                            std::vector<double> targets) {
	ModeMonitor m;
	m.axis = section.axis;
	m.polarity = section.polarity;
	m.fixed_index = section.fixed_index;
	m.range1 = section.range1;
	m.range2 = section.range2;
	m.delta1 = section.delta1;
	m.delta2 = section.delta2;
	m.delta_normal = section.delta_normal;
	m.targets = std::move(targets);
	m.modes_by_freq = std::move(modes_by_freq);
	return m;
}

const VectorModeProfile& ModeForFrequency(const ModeMonitor& m, double f) {
	auto it = m.modes_by_freq.find(f);
	if (it != m.modes_by_freq.end()) return it->second;
	// nearest within a tight relative tolerance
	const VectorModeProfile* best = nullptr;
	double best_d = 0.0;
	for (const auto& [k, v] : m.modes_by_freq) {
		const double d = std::abs(k - f);
		if (!best || d < best_d) { best = &v; best_d = d; }
	}
	if (best && best_d <= std::max(1.0, std::abs(f)) * 1e-12) return *best;
	if (best) return *best;  // fall back to the closest (Julia falls back to monitor.mode)
	throw std::runtime_error("monitor has no modes");
}

namespace {

// Extract the transverse (range1 x range2) slice on the monitor's fixed plane.
MatrixXcd extract_section(const Grid3<cdouble>& field, const ModeMonitor& m) {
	const int n1 = static_cast<int>(m.range1.size());
	const int n2 = static_cast<int>(m.range2.size());
	MatrixXcd out(n1, n2);
	for (int b = 0; b < n2; ++b)
		for (int a = 0; a < n1; ++a) {
			const int r1 = static_cast<int>(m.range1.lo) + a;
			const int r2 = static_cast<int>(m.range2.lo) + b;
			if (m.axis == Axis::Z)
				out(a, b) = field(r1, r2, static_cast<int>(m.fixed_index));
			else if (m.axis == Axis::X)
				out(a, b) = field(static_cast<int>(m.fixed_index), r1, r2);
			else
				out(a, b) = field(r1, static_cast<int>(m.fixed_index), r2);
		}
	return out;
}

// H averaged from the two Yee planes straddling the E plane (== ExtractHFieldSectionAtEPlane).
MatrixXcd extract_h_at_e(const Grid3<cdouble>& field, const ModeMonitor& m) {
	const long n = m.fixed_index;
	const int n1 = static_cast<int>(m.range1.size());
	const int n2 = static_cast<int>(m.range2.size());
	MatrixXcd out(n1, n2);
	for (int b = 0; b < n2; ++b)
		for (int a = 0; a < n1; ++a) {
			const int r1 = static_cast<int>(m.range1.lo) + a;
			const int r2 = static_cast<int>(m.range2.lo) + b;
			cdouble prev, cur;
			if (m.axis == Axis::X) {
				prev = field(static_cast<int>(n - 1), r1, r2);
				cur = field(static_cast<int>(n), r1, r2);
			} else if (m.axis == Axis::Y) {
				prev = field(r1, static_cast<int>(n - 1), r2);
				cur = field(r1, static_cast<int>(n), r2);
			} else {
				prev = field(r1, r2, static_cast<int>(n - 1));
				cur = field(r1, r2, static_cast<int>(n));
			}
			out(a, b) = 0.5 * (prev + cur);
		}
	return out;
}

// == ModeMagneticFieldsAtEPlane
std::array<MatrixXcd, 3> mode_h_at_e_plane(const VectorModeProfile& mode,
                                           double delta_normal) {
	const cdouble p2c = std::exp(cdouble(0.0, 1.0) *
	                             static_cast<double>(mode.polarity) * mode.beta *
	                             delta_normal);
	const cdouble interp = 0.5 * (cdouble(1.0, 0.0) + p2c);
	return {mode.Hx * interp, mode.Hy * interp, mode.Hz * interp};
}

// == VectorOverlapTerms -> (electric_term, magnetic_term)
std::pair<cdouble, cdouble> vector_overlap_terms(
    Axis axis, int polarity, const std::array<MatrixXcd, 3>& E,
    const std::array<MatrixXcd, 3>& H, const VectorModeProfile& mode,
    double delta1, double delta2, const std::array<MatrixXcd, 3>& Hm) {
	auto S = [](const MatrixXcd& A, const MatrixXcd& B) {  // sum(A .* conj(B))
		return (A.array() * B.array().conjugate()).sum();
	};
	cdouble e_int, m_int;
	if (axis == Axis::Z) {
		e_int = S(E[0], Hm[1]) - S(E[1], Hm[0]);
		m_int = S(H[1], mode.Ex) - S(H[0], mode.Ey);  // conj(mode.E).*H
	} else if (axis == Axis::X) {
		e_int = S(E[1], Hm[2]) - S(E[2], Hm[1]);
		m_int = S(H[2], mode.Ey) - S(H[1], mode.Ez);
	} else {
		e_int = S(E[2], Hm[0]) - S(E[0], Hm[2]);
		m_int = S(H[0], mode.Ez) - S(H[2], mode.Ex);
	}
	const double area = delta1 * delta2;
	return {static_cast<double>(polarity) * e_int * area,
	        static_cast<double>(polarity) * m_int * area};
}

// == DirectionalModalAmplitudes -> (forward, backward)
std::pair<cdouble, cdouble> directional_modal_amplitudes(
    Axis axis, int polarity, const std::array<MatrixXcd, 3>& E,
    const std::array<MatrixXcd, 3>& H, const VectorModeProfile& mode,
    double delta1, double delta2, double delta_normal) {
	const auto Hm = mode_h_at_e_plane(mode, delta_normal);
	const auto [e_term, m_term] =
	    vector_overlap_terms(axis, polarity, E, H, mode, delta1, delta2, Hm);
	const std::array<MatrixXcd, 3> mode_E{mode.Ex, mode.Ey, mode.Ez};
	const auto [self_e, self_m] = vector_overlap_terms(
	    axis, polarity, mode_E, Hm, mode, delta1, delta2, Hm);
	const cdouble e_amp = e_term / self_e;
	const cdouble m_amp = m_term / self_m;
	return {0.5 * (e_amp + m_amp), 0.5 * (e_amp - m_amp)};
}

}  // namespace

cdouble ModalOverlapAtFrequency(const ModeMonitor& monitor, double freq_hz,
                                const FreqFields& fields) {
	const VectorModeProfile& mode = ModeForFrequency(monitor, freq_hz);
	const std::array<MatrixXcd, 3> Esec{extract_section(fields.E[0], monitor),
	                                    extract_section(fields.E[1], monitor),
	                                    extract_section(fields.E[2], monitor)};
	const std::array<MatrixXcd, 3> Hsec{extract_h_at_e(fields.H[0], monitor),
	                                    extract_h_at_e(fields.H[1], monitor),
	                                    extract_h_at_e(fields.H[2], monitor)};
	const auto [forward, backward] = directional_modal_amplitudes(
	    monitor.axis, monitor.polarity, Esec, Hsec, mode, monitor.delta1,
	    monitor.delta2, monitor.delta_normal);
	(void)backward;
	return forward;
}

std::pair<cdouble, cdouble> ModalAmplitudesAtFrequency(const ModeMonitor& monitor,
                                                       double freq_hz,
                                                       const FreqFields& fields) {
	const VectorModeProfile& mode = ModeForFrequency(monitor, freq_hz);
	const std::array<MatrixXcd, 3> Esec{extract_section(fields.E[0], monitor),
	                                    extract_section(fields.E[1], monitor),
	                                    extract_section(fields.E[2], monitor)};
	const std::array<MatrixXcd, 3> Hsec{extract_h_at_e(fields.H[0], monitor),
	                                    extract_h_at_e(fields.H[1], monitor),
	                                    extract_h_at_e(fields.H[2], monitor)};
	return directional_modal_amplitudes(monitor.axis, monitor.polarity, Esec, Hsec,
	                                    mode, monitor.delta1, monitor.delta2,
	                                    monitor.delta_normal);
}

VectorModeProfile CanonicalizeVectorModePhase(const VectorModeProfile& mode) {
	return canonicalize_phase(mode);
}

SourceNormalization ComputeSourceNormalization(
    const std::vector<double>& freqs,
    const std::map<std::string, ModeMonitor>& monitors,
    const FreqsField& freqs_field,
    const std::vector<std::string>& monitor_names) {
	SourceNormalization sn;
	sn.monitor_names = monitor_names;
	sn.freqs = freqs;
	sn.power.assign(freqs.size(), 0.0);

	for (const auto& name : monitor_names) {
		const ModeMonitor& mon = monitors.at(name);
		std::vector<cdouble> overlaps;
		std::vector<double> powers;
		for (std::size_t k = 0; k < freqs.size(); ++k) {
			const cdouble o =
			    ModalOverlapAtFrequency(mon, freqs[k], freqs_field.at(freqs[k]));
			const double p = std::norm(o);  // abs2
			overlaps.push_back(o);
			powers.push_back(p);
			sn.power[k] += p;
		}
		sn.overlap_by_monitor[name] = std::move(overlaps);
		sn.power_by_monitor[name] = std::move(powers);
	}
	return sn;
}

ModalOutputs ComputeModalOutputs(
    const std::vector<double>& freqs,
    const std::map<std::string, ModeMonitor>& monitors,
    const FreqsField& freqs_field,
    const std::vector<std::string>& monitor_names,
    const SourceNormalization* source_normalization,
    const std::vector<std::string>& input_monitor_names) {
	ModalOutputs out;
	out.freqs = freqs;

	for (const auto& name : monitor_names) {
		const ModeMonitor& mon = monitors.at(name);
		std::vector<cdouble> c_values;
		std::vector<double> modal_powers;
		for (double f : freqs) {
			const cdouble o = ModalOverlapAtFrequency(mon, f, freqs_field.at(f));
			c_values.push_back(o);
			modal_powers.push_back(std::norm(o));
		}
		out.overlap[name] = std::move(c_values);
		out.modal_power[name] = std::move(modal_powers);
	}

	out.source_normalization =
	    source_normalization
	        ? *source_normalization
	        : ComputeSourceNormalization(freqs, monitors, freqs_field,
	                                     input_monitor_names);

	for (const auto& name : monitor_names) {
		std::vector<double> J(freqs.size());
		for (std::size_t k = 0; k < freqs.size(); ++k)
			J[k] = out.modal_power.at(name)[k] / out.source_normalization.power[k];
		out.J[name] = std::move(J);
	}
	return out;
}

// ---- adjoint vector modal source (data structures only) -----------------

AdjointVectorModalSource MakeAdjointVectorModalSource(
    const ModeMonitor& section, const std::vector<double>& freqs,
    const std::map<double, VectorModeProfile>& mode_by_freq,
    const std::vector<cdouble>& adjoint_weights, int polarity_override,
    double J0, int phase_sign) {
	AdjointVectorModalSource s;
	s.section = section;
	s.freqs = freqs;
	s.mode_by_freq = mode_by_freq;
	s.adjoint_weights = adjoint_weights;
	s.axis = section.axis;
	s.fixed_index = section.fixed_index;
	s.range1 = section.range1;
	s.range2 = section.range2;
	s.polarity = polarity_override != 0 ? polarity_override : -section.polarity;
	s.J0 = J0;
	s.phase_sign = phase_sign;
	return s;
}

const VectorModeProfile& ModeForAdjointSourceFrequency(
    const AdjointVectorModalSource& source, double freq_hz) {
	auto it = source.mode_by_freq.find(freq_hz);
	if (it != source.mode_by_freq.end()) return it->second;

	const auto* best = &source.mode_by_freq.begin()->second;
	double best_d = std::numeric_limits<double>::infinity();
	for (const auto& [f, mode] : source.mode_by_freq) {
		const double d = std::abs(f - freq_hz);
		if (d < best_d) {
			best_d = d;
			best = &mode;
		}
	}
	return *best;
}

}  // namespace lucuma::vulkan
