// == auxiliaries/funcs.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auxiliaries/funcs.hpp"

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <fstream>
#include <regex>
#include <sstream>

namespace lucuma::vulkan {

// ---------------------------------------------------------------------------

// Shortest round-trip decimal, with a trailing ".0" on whole numbers so the
// value still reads as a float (Julia's `print(::Float64)` does the same).
std::string julia_repr(double x) {
	char buf[32];
	auto [ptr, ec] = std::to_chars(buf, buf + sizeof buf, x);
	std::string s(buf, ptr);
	if (ec == std::errc{} && s.find_first_of(".eEnN") == std::string::npos)
		s += ".0";
	return s;
}

// ---------------------------------------------------------------------------
// TOML access
// ---------------------------------------------------------------------------

Params LoadParams(const std::filesystem::path& path) {
	return toml::parse_file(path.string());
}

[[noreturn]] static void missing(std::string_view t, std::string_view k) {
	throw std::runtime_error("missing TOML key [" + std::string(t) + "]." +
	                         std::string(k));
}

double TomlF64(const Params& p, std::string_view t, std::string_view k) {
	if (auto v = p[t][k].value<double>()) return *v;
	missing(t, k);
}

long TomlInt(const Params& p, std::string_view t, std::string_view k) {
	if (auto v = p[t][k].value<int64_t>()) return static_cast<long>(*v);
	missing(t, k);
}

std::string TomlStr(const Params& p, std::string_view t, std::string_view k) {
	if (auto v = p[t][k].value<std::string>()) return *v;
	missing(t, k);
}

double TomlNum(const Params& p, std::string_view t, std::string_view k) {
	if (auto v = p[t][k].value<double>()) return *v;
	if (auto v = p[t][k].value<int64_t>()) return static_cast<double>(*v);
	missing(t, k);
}

std::vector<double> TomlF64Array(const Params& p, std::string_view t,
                                 std::string_view k) {
	std::vector<double> out;
	auto arr = p[t][k].as_array();
	if (!arr) missing(t, k);
	for (auto&& e : *arr) out.push_back(e.value<double>().value());
	return out;
}

std::vector<long> TomlIntArray(const Params& p, std::string_view t,
                               std::string_view k) {
	std::vector<long> out;
	auto arr = p[t][k].as_array();
	if (!arr) missing(t, k);
	for (auto&& e : *arr)
		out.push_back(static_cast<long>(e.value<int64_t>().value()));
	return out;
}

std::vector<std::string> TomlStrArray(const Params& p, std::string_view t,
                                      std::string_view k) {
	std::vector<std::string> out;
	auto arr = p[t][k].as_array();
	if (!arr) missing(t, k);
	for (auto&& e : *arr) out.push_back(e.value<std::string>().value());
	return out;
}

std::array<double, 2> TomlPair(const Params& p, std::string_view t,
                               std::string_view k) {
	auto arr = p[t][k].as_array();
	if (!arr || arr->size() != 2) missing(t, k);
	return {(*arr)[0].value<double>().value(),
	        (*arr)[1].value<double>().value()};
}

// ---------------------------------------------------------------------------
// Line-wise TOML editors
// ---------------------------------------------------------------------------

static std::vector<std::string> read_lines(const std::filesystem::path& path) {
	std::ifstream in(path);
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(in, line)) {
		if (!line.empty() && line.back() == '\r') line.pop_back();
		lines.push_back(std::move(line));
	}
	return lines;
}

static void write_lines(const std::filesystem::path& path,
                        const std::vector<std::string>& lines) {
	std::ofstream out(path, std::ios::binary | std::ios::trunc);
	for (const auto& l : lines) out << l << '\n';
}

// Replace, on the first matching line only, the text between capture groups 1
// and 2 of `pattern` with `replacement`.
static void edit_first(const std::filesystem::path& path,
                       const std::regex& pattern,
                       const std::string& replacement) {
	auto lines = read_lines(path);
	for (auto& line : lines) {
		std::smatch m;
		if (std::regex_search(line, m, pattern)) {
			line = m[1].str() + replacement + m[2].str();
			break;
		}
	}
	write_lines(path, lines);
}

void UpdateStage(const std::filesystem::path& path, const std::string& s) {
	edit_first(path, std::regex(R"rx(^(\s*stage\s*=\s*)"[^"]*"(.*)$)rx"),
	           "\"" + s + "\"");
}

void UpdateBetaRange(const std::filesystem::path& path, int beta) {
	edit_first(path, std::regex(R"(^(\s*beta\s*=\s*)-?\d+(.*)$)"),
	           std::to_string(beta));
}

void UpdateIt(const std::filesystem::path& path, int it) {
	edit_first(path, std::regex(R"(^(\s*it\s*=\s*)-?\d+(.*)$)"),
	           std::to_string(it));
}

void UpdateNeighbours(const std::filesystem::path& path,
                      const std::vector<int>& n) {
	std::string list = "[";
	for (std::size_t i = 0; i < n.size(); ++i)
		list += (i ? "," : "") + std::to_string(n[i]);
	list += "]";
	edit_first(path, std::regex(R"(^(\s*neighbours\s*=\s*)\[[^\]]*\](.*)$)"),
	           list);
}

// ---------------------------------------------------------------------------
// P array
// ---------------------------------------------------------------------------

void InitP(int nx, int ny, int nz, const std::filesystem::path& path,
           bool p_ones) {
	std::filesystem::create_directories(path.parent_path());
	std::ofstream out(path, std::ios::binary | std::ios::trunc);
	const std::size_t n =
	    static_cast<std::size_t>(nx) * ny * nz;
	if (p_ones) {
		for (std::size_t i = 0; i < n; ++i) out << "1.0\n";
	} else {
		// rand() path is only used by optimization.jl (out of scope); keep a
		// deterministic-ish stand-in so the file shape is still correct.
		for (std::size_t i = 0; i < n; ++i) out << "0.5\n";
	}
}

void SaveP(const std::vector<double>& p_flat,
           const std::filesystem::path& path) {
	std::filesystem::create_directories(path.parent_path());
	std::ofstream out(path, std::ios::binary | std::ios::trunc);
	for (double v : p_flat) out << julia_repr(v) << '\n';
}

std::vector<double> LoadP(int nx, int ny, int nz,
                          const std::filesystem::path& path) {
	std::ifstream in(path);
	if (!in) throw std::runtime_error("cannot open P file: " + path.string());

	std::vector<double> flat;
	std::string tok;
	// readdlm(path, ',', Float64): comma- and/or newline-separated scalars.
	std::string content((std::istreambuf_iterator<char>(in)),
	                    std::istreambuf_iterator<char>());
	for (char& c : content)
		if (c == ',' || c == '\r') c = '\n';
	std::istringstream ss(content);
	while (ss >> tok) flat.push_back(std::stod(tok));

	const std::size_t expected =
	    static_cast<std::size_t>(nx) * ny * nz;
	if (flat.size() != expected)
		throw std::runtime_error("P file " + path.string() + " has " +
		                         std::to_string(flat.size()) +
		                         " values, expected " +
		                         std::to_string(expected));
	return flat;  // already column-major
}

// ---------------------------------------------------------------------------

std::array<long, 2> ValuesToIndx(double b0, double b1, double step) {
	return {julia_round(b0 / step) + 1, julia_round(b1 / step)};
}

std::vector<std::string> unite(std::vector<std::string> a,
                               const std::vector<std::string>& b) {
	for (const auto& s : b)
		if (std::find(a.begin(), a.end(), s) == a.end()) a.push_back(s);
	return a;
}

}  // namespace lucuma::vulkan
