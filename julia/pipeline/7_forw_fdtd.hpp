// == pipeline/7_forw_fdtd.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "auxiliaries/modes.hpp"
#include "fdtd/backends/i_backend.hpp"
#include "fdtd/fw_sim.hpp"
#include "fdtd/params.hpp"

namespace lucuma::julia {

// == BuildOutputPortData entry: everything the adjoint stage needs per monitor.
struct OutputPort {
	ModeMonitor section;              // == "monitor" / "section"
	std::vector<double> freqs;
	std::vector<cdouble> c_e_overlap;
	std::vector<double> modal_power;
	std::vector<double> J;
	std::vector<double> source_power;
	std::vector<double> targets;
	std::vector<double> loss_terms;   // (J - target)^2
	double loss = 0.0;
	std::vector<cdouble> adjoint_weight_J;   // c_e_overlap ./ source_power
	std::vector<double> transmission;        // == J
	Axis axis = Axis::X;
	int forward_polarity = 1;
	int adjoint_polarity = -1;
	std::string forward_direction, adjoint_direction;
};

// == WrapAdjointSourcesForFDTD entry.
struct WrappedAdjointSource {
	AdjointVectorModalSource source;
	std::vector<cdouble> c_e_overlap;
	std::vector<cdouble> adjoint_weight_J;
	std::vector<double> J;
	std::vector<double> targets;
	std::vector<double> loss_terms;
	double loss = 0.0;
	std::vector<double> source_power;
	std::string direction;
	Axis axis = Axis::X;
	int polarity = 0;
};

struct ForwardResult {
	double loss = 0.0;
	double transmittance = 0.0;
	std::vector<double> freqs;
	std::map<std::string, std::vector<double>> J;           // per output monitor
	std::map<std::string, std::vector<double>> loss_terms;  // (J - target)^2
	std::map<std::string, std::vector<double>> targets;
	SourceNormalization source_normalization;

	FreqsField fields;                                      // == "fields"
	std::map<std::string, OutputPort> output_ports;
	std::map<std::string, WrappedAdjointSource> adjoint_sources;
};

double TotalLoss(const ForwardResult& r,
                 const std::vector<std::string>& monitors_out);
double TotalTransmittance(const ForwardResult& r,
                          const std::vector<std::string>& monitors_out);

std::string AxisPolarityToDirection(Axis axis, int polarity);

std::map<std::string, OutputPort> BuildOutputPortData(
    const FDTDParams& params, const ModalOutputs& modal_outputs,
    const std::vector<std::string>& monitors_out);

std::map<std::string, AdjointVectorModalSource> BuildAdjointSourceFunctors(
    const std::map<std::string, OutputPort>& output_ports,
    const std::vector<double>& freqs, double J0 = 1.0, int phase_sign = +1);

std::map<std::string, WrappedAdjointSource> WrapAdjointSourcesForFDTD(
    const std::map<std::string, AdjointVectorModalSource>& functors,
    const std::map<std::string, OutputPort>& output_ports);

// Refresh design coeffs, run forward FDTD + DFT, reduce to transmission / loss / ports.
template <class T>
ForwardResult RunForwardFDTD(
    FDTDParams& params, IFdtdBackend<T>& backend, const std::vector<double>& PPP,
    const std::vector<std::string>& monitors_in,
    const std::vector<std::string>& monitors_out,
    const SourceNormalization& source_normalization,
    const std::filesystem::path& vis_path = {}, const std::string& proj = {},
    const std::string& additional = {}, const std::string& snapshot_plane = "xy",
    const std::string& snapshot_field = "Ey", int step = 20,
    bool save_result = false, double adjoint_J0 = 1.0,
    int adjoint_phase_sign = +1) {
	if (save_result && proj == "nom") {
		std::error_code ec;
		std::filesystem::remove_all(vis_path, ec);
	}

	params.opt_region_values = PPP;
	backend.updateOptRegionCoeffs(params);

	const FreqsField ff = FwFDTDSimulation<T>(
	    params, backend, vis_path / proj, additional, snapshot_plane,
	    snapshot_field, step, save_result);

	const ModalOutputs mo = ComputeModalOutputs(params.freqs, params.monitors, ff,
	                                            monitors_out, &source_normalization,
	                                            monitors_in);

	ForwardResult r;
	r.freqs = params.freqs;
	r.fields = ff;
	r.source_normalization = mo.source_normalization;
	r.output_ports = BuildOutputPortData(params, mo, monitors_out);

	const auto functors = BuildAdjointSourceFunctors(
	    r.output_ports, params.freqs, adjoint_J0, adjoint_phase_sign);
	r.adjoint_sources = WrapAdjointSourcesForFDTD(functors, r.output_ports);

	for (const auto& name : monitors_out) {
		const OutputPort& port = r.output_ports.at(name);
		r.J[name] = port.J;
		r.targets[name] = port.targets;
		r.loss_terms[name] = port.loss_terms;
	}

	r.loss = TotalLoss(r, monitors_out);
	r.transmittance = TotalTransmittance(r, monitors_out);
	return r;
}

}  // namespace lucuma::julia
