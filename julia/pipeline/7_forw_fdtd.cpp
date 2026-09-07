// == pipeline/7_forw_fdtd.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pipeline/7_forw_fdtd.hpp"

#include <numeric>

namespace lucuma::julia {

double TotalLoss(const ForwardResult& r,
                 const std::vector<std::string>& monitors_out) {
	double s = 0.0;
	for (const auto& name : monitors_out) {
		const auto& lt = r.loss_terms.at(name);
		s += std::accumulate(lt.begin(), lt.end(), 0.0);
	}
	return s;
}

double TotalTransmittance(const ForwardResult& r,
                          const std::vector<std::string>& monitors_out) {
	if (r.freqs.empty()) return 0.0;
	std::vector<double> tr(r.freqs.size(), 0.0);
	for (const auto& name : monitors_out) {
		const auto& J = r.J.at(name);
		for (std::size_t k = 0; k < tr.size(); ++k) tr[k] += J[k];
	}
	return std::accumulate(tr.begin(), tr.end(), 0.0) /
	       static_cast<double>(tr.size());
}

std::string AxisPolarityToDirection(Axis axis, int polarity) {
	const char* a = axis == Axis::X ? "x" : axis == Axis::Y ? "y" : "z";
	return std::string(polarity >= 0 ? "+" : "-") + a;
}

std::map<std::string, OutputPort> BuildOutputPortData(
    const FDTDParams& params, const ModalOutputs& modal_outputs,
    const std::vector<std::string>& monitors_out) {
	std::map<std::string, OutputPort> output_ports;
	const std::vector<double> freqs = modal_outputs.freqs;
	const std::vector<double>& source_power =
	    modal_outputs.source_normalization.power;

	for (const auto& name : monitors_out) {
		const ModeMonitor& monitor = params.monitors.at(name);

		OutputPort port;
		port.section = monitor;
		port.freqs = freqs;
		port.c_e_overlap = modal_outputs.overlap.at(name);
		port.modal_power = modal_outputs.modal_power.at(name);
		port.J = modal_outputs.J.at(name);
		port.source_power = source_power;

		port.targets = monitor.targets.empty()
		                   ? std::vector<double>(freqs.size(), 1.0)
		                   : monitor.targets;

		port.loss_terms.resize(port.J.size());
		for (std::size_t k = 0; k < port.J.size(); ++k)
			port.loss_terms[k] =
			    (port.J[k] - port.targets[k]) * (port.J[k] - port.targets[k]);
		port.loss = std::accumulate(port.loss_terms.begin(),
		                            port.loss_terms.end(), 0.0);

		port.adjoint_weight_J.resize(port.c_e_overlap.size());
		for (std::size_t k = 0; k < port.c_e_overlap.size(); ++k)
			port.adjoint_weight_J[k] = port.c_e_overlap[k] / source_power[k];

		port.transmission = port.J;
		port.axis = monitor.axis;
		port.forward_polarity = monitor.polarity;
		port.adjoint_polarity = -monitor.polarity;
		port.forward_direction =
		    AxisPolarityToDirection(monitor.axis, monitor.polarity);
		port.adjoint_direction =
		    AxisPolarityToDirection(monitor.axis, -monitor.polarity);

		output_ports.emplace(name, std::move(port));
	}
	return output_ports;
}

std::map<std::string, AdjointVectorModalSource> BuildAdjointSourceFunctors(
    const std::map<std::string, OutputPort>& output_ports,
    const std::vector<double>& freqs, double J0, int phase_sign) {
	std::map<std::string, AdjointVectorModalSource> functors;
	for (const auto& [port_name, port] : output_ports)
		functors.emplace(
		    port_name,
		    MakeAdjointVectorModalSource(port.section, freqs,
		                                 port.section.modes_by_freq,
		                                 port.adjoint_weight_J,
		                                 port.adjoint_polarity, J0, phase_sign));
	return functors;
}

std::map<std::string, WrappedAdjointSource> WrapAdjointSourcesForFDTD(
    const std::map<std::string, AdjointVectorModalSource>& functors,
    const std::map<std::string, OutputPort>& output_ports) {
	std::map<std::string, WrappedAdjointSource> wrapped;
	for (const auto& [port_name, g] : functors) {
		const OutputPort& port = output_ports.at(port_name);
		WrappedAdjointSource w;
		w.source = g;
		w.c_e_overlap = port.c_e_overlap;
		w.adjoint_weight_J = port.adjoint_weight_J;
		w.J = port.J;
		w.targets = port.targets;
		w.loss_terms = port.loss_terms;
		w.loss = port.loss;
		w.source_power = port.source_power;
		w.direction = port.adjoint_direction;
		w.axis = port.axis;
		w.polarity = port.adjoint_polarity;
		wrapped.emplace(port_name, std::move(w));
	}
	return wrapped;
}

}  // namespace lucuma::julia
