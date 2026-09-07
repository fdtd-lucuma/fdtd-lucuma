// == auxiliaries/modes.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <complex>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <Eigen/Dense>

#include "auxiliaries/array.hpp"
#include "auxiliaries/funcs.hpp"

namespace lucuma::vulkan {

using cdouble = std::complex<double>;

// A 1-based inclusive index range (Julia `lo:hi`).
struct IdxRange {
	long lo = 1;
	long hi = 0;
	long size() const { return hi - lo + 1; }
	long first() const { return lo; }
	long last() const { return hi; }
};

// modes.jl PortSection
struct PortSection {
	Mat2<double> epsr;   // 2-D transverse permittivity (rows = range1, cols = range2)
	Axis axis = Axis::X;
	int polarity = 1;
	long fixed_index = 0;
	IdxRange range1;
	IdxRange range2;
	double delta1 = 0.0;
	double delta2 = 0.0;
	double delta_normal = 0.0;
};

// ---- grid helpers -----------------------------------------------------------

IdxRange ValueRangeToIndexRange(double b0, double b1, double step);
long CenterIndex(const IdxRange& r);
IdxRange ExpandRange(const IdxRange& r, long pad, long N);
Axis DirectionAxis(const std::string& direction);
int DirectionPolarity(const std::string& direction);
// (dy,dz) for axis x, (dx,dz) for y, (dx,dy) for z.
std::pair<double, double> AxisDeltas(const Params& params, Axis axis);

// ---- epsilon grid + cross section ----------------------------------------

// waveguides_names default {"win","wout"} at the call site in 3_wg_source.
// If p_flat is non-empty it is the flattened (column-major, nxP*nyP*nzP)
// design density and is written into the opt_name box as
// eps_bg + P*(eps_fg - eps_bg).
Grid3<double> BuildRelativePermittivityGrid(
    const Params& params, const Params& device,
    const std::vector<std::string>& waveguides_names,
    const std::vector<double>& p_flat = {}, int nxP = 0, int nyP = 0,
    int nzP = 0, const std::string& opt_name = "P");

PortSection ExtractPortSection(const Grid3<double>& eps_grid, const BBox& bbox,
                               const Params& params, long transverse_pad = 0,
                               const std::string& direction_override = "");

// ---- full-vector mode solver ---------------------------------------------

// Default speed of light / vacuum permittivity used by the mode solver. These
// are the `c0` / `eps0` keyword-argument defaults of SolveVectorModes in
// modes.jl (lines 491, 566) and the literal 299792458.0 in
// InterpolateTrackedMode (line 694); nothing in the pipeline overrides them.
// (kEps0 equals params.toml [material].eps0.)
inline constexpr double kC0 = 299792458.0;
inline constexpr double kEps0 = 8.854187817e-12;

// modes.jl VectorModeProfile. Field matrices are in global orientation, stored
// in the section's (range1, range2) index order.
struct VectorModeProfile {
	Eigen::MatrixXcd Ex, Ey, Ez, Hx, Hy, Hz;
	cdouble neff{}, beta{}, eigenvalue{};
	double omega = 0.0;
	Axis axis = Axis::X;
	int polarity = 1;
};

// nmodes eigenpairs of largest real part, reconstructed E/H, power-normalised,
// ordered by descending real(neff) (== SolveVectorModes). c0 / eps0 mirror the
// modes.jl keyword defaults.
std::vector<VectorModeProfile> SolveVectorModes(const PortSection& section,
                                                double omega, int nmodes = 4,
                                                double c0 = kC0,
                                                double eps0 = kEps0);
VectorModeProfile SolveVectorMode(const PortSection& section, double omega,
                                  int nmodes = 4, int mode_index = 1,
                                  double c0 = kC0, double eps0 = kEps0);

// == CanonicalizeVectorModePhase : fix a solved mode's global phase so its
// largest-magnitude transverse E sample is real and positive.
VectorModeProfile CanonicalizeVectorModePhase(const VectorModeProfile& mode);

// ---- tracked mode family (broadband source) -----------------------------

struct TrackedModeFamily {
	PortSection section;
	std::vector<double> frequencies;         // Hz, ascending
	std::vector<VectorModeProfile> modes;    // one per frequency, phase-aligned
};

std::vector<double> ChebyshevFrequencySamples(double fmin, double fmax, int count);
TrackedModeFamily BuildTrackedModeFamily(const PortSection& section,
                                         const std::vector<double>& frequencies,
                                         int nmodes = 4,
                                         int initial_mode_index = 1);
VectorModeProfile InterpolateTrackedMode(const TrackedModeFamily& family,
                                         double frequency);

// ---- mode monitors + modal overlap -------------------------------------

// modes.jl ModeMonitor
struct ModeMonitor {
	Axis axis = Axis::X;
	int polarity = 1;
	long fixed_index = 0;
	IdxRange range1, range2;
	double delta1 = 0.0, delta2 = 0.0, delta_normal = 0.0;
	std::vector<double> targets;
	std::map<double, VectorModeProfile> modes_by_freq;
};
ModeMonitor MakeModeMonitor(const PortSection& section,
                            std::map<double, VectorModeProfile> modes_by_freq,
                            std::vector<double> targets = {});
const VectorModeProfile& ModeForFrequency(const ModeMonitor& m, double freq_hz);

// ---- adjoint vector modal source (data structures only) -----------------

// sources.jl AdjointVectorModalSource (built and stored only; no adjoint loop).
struct AdjointVectorModalSource {
	ModeMonitor section;                            // the originating monitor
	std::vector<double> freqs;                      // Hz
	std::map<double, VectorModeProfile> mode_by_freq;
	std::vector<cdouble> adjoint_weights;
	Axis axis = Axis::X;
	long fixed_index = 0;
	IdxRange range1, range2;
	int polarity = -1;
	double J0 = 1.0;
	int phase_sign = +1;
};

// == AdjointVectorModalSource ctor. polarity_override == 0 means -section.polarity.
AdjointVectorModalSource MakeAdjointVectorModalSource(
    const ModeMonitor& section, const std::vector<double>& freqs,
    const std::map<double, VectorModeProfile>& mode_by_freq,
    const std::vector<cdouble>& adjoint_weights, int polarity_override = 0,
    double J0 = 1.0, int phase_sign = +1);

// == ModeForAdjointSourceFrequency : exact key, else nearest by |Δf|.
const VectorModeProfile& ModeForAdjointSourceFrequency(
    const AdjointVectorModalSource& source, double freq_hz);

// Per-frequency complex field volumes accumulated by the running DFT.
struct FreqFields {
	Grid3<cdouble> E[3];  // Ex, Ey, Ez
	Grid3<cdouble> H[3];  // Hx, Hy, Hz
};
using FreqsField = std::map<double, FreqFields>;

// Forward modal amplitude of the DFT fields at `freq_hz` through `monitor`.
cdouble ModalOverlapAtFrequency(const ModeMonitor& monitor, double freq_hz,
                                const FreqFields& fields);

// == ModalAmplitudesAtFrequency : (forward, backward) directional modal
// amplitudes of the DFT fields at `freq_hz` through `monitor`. `forward`
// travels along +monitor.polarity, `backward` against it.
std::pair<cdouble, cdouble> ModalAmplitudesAtFrequency(const ModeMonitor& monitor,
                                                       double freq_hz,
                                                       const FreqFields& fields);

struct SourceNormalization {
	std::vector<std::string> monitor_names;
	std::vector<double> freqs;
	std::map<std::string, std::vector<cdouble>> overlap_by_monitor;
	std::map<std::string, std::vector<double>> power_by_monitor;
	std::vector<double> power;  // total incident power per frequency
};

struct ModalOutputs {
	std::vector<double> freqs;
	std::map<std::string, std::vector<cdouble>> overlap;
	std::map<std::string, std::vector<double>> modal_power;
	std::map<std::string, std::vector<double>> J;  // transmission per frequency
	SourceNormalization source_normalization;
};

// monitors: name -> ModeMonitor ; freqs: the optimisation frequencies.
SourceNormalization ComputeSourceNormalization(
    const std::vector<double>& freqs,
    const std::map<std::string, ModeMonitor>& monitors,
    const FreqsField& freqs_field, const std::vector<std::string>& monitor_names);

ModalOutputs ComputeModalOutputs(
    const std::vector<double>& freqs,
    const std::map<std::string, ModeMonitor>& monitors,
    const FreqsField& freqs_field,
    const std::vector<std::string>& monitor_names,
    const SourceNormalization* source_normalization,
    const std::vector<std::string>& input_monitor_names);

}  // namespace lucuma::vulkan
