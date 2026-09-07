// == auxiliaries/funcs.jl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Config loading, TOML line-wise editors, P.csv (de)serialisation and the
// index helpers shared across the pipeline.

#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "toml.hpp"

namespace lucuma::julia {

using Params = toml::table;

// Axis codes match the integer `axis` used by the ported CUDA kernels.
enum class Axis { X = 1, Y = 2, Z = 3 };

// A device bounding box: name + three 1-based inclusive index ranges
// {lo, hi} for x/y/z, plus an optional "+x"/"-y"/... direction string.
// (Julia represents this as a `Vector{Any}`.)
struct BBox {
	std::string name;
	std::array<std::array<long, 2>, 3> r{};
	std::optional<std::string> direction;
};

// ---------------------------------------------------------------------------
// Numeric helpers matching Julia semantics
// ---------------------------------------------------------------------------

// Julia `round(Int, x)` uses round-half-to-even.
inline long julia_round(double x) { return std::lround(std::nearbyint(x)); }

// Shortest round-trip repr, but keep a trailing ".0" for whole numbers the way
// Julia's `print(::Float64)` does (used for TOML write-back and P.csv).
std::string julia_repr(double x);

// ---------------------------------------------------------------------------
// TOML access (mirrors indexing into the Dict returned by TOML.parsefile)
// ---------------------------------------------------------------------------

Params LoadParams(const std::filesystem::path& path);

double              TomlF64(const Params&, std::string_view table, std::string_view key);
long                TomlInt(const Params&, std::string_view table, std::string_view key);
// Reads an int- or float-valued node as double (TOML has no implicit widening).
double              TomlNum(const Params&, std::string_view table, std::string_view key);
std::string         TomlStr(const Params&, std::string_view table, std::string_view key);
std::vector<double> TomlF64Array(const Params&, std::string_view table, std::string_view key);
std::vector<long>   TomlIntArray(const Params&, std::string_view table, std::string_view key);
std::vector<std::string> TomlStrArray(const Params&, std::string_view table, std::string_view key);

// device[name][coord] -> 2-element [m] vector (e.g. device["win"]["x"])
std::array<double, 2> TomlPair(const Params&, std::string_view table, std::string_view key);

// ---------------------------------------------------------------------------
// Line-wise TOML editors (preserve comments; mirror the regex edits in funcs.jl)
// ---------------------------------------------------------------------------

void UpdateStage(const std::filesystem::path& path, const std::string& new_stage);
void UpdateBetaRange(const std::filesystem::path& path, int new_beta);
void UpdateIt(const std::filesystem::path& path, int new_it);
void UpdateNeighbours(const std::filesystem::path& path, const std::vector<int>& neighbours);

// ---------------------------------------------------------------------------
// Optimisation-region density array P  (flat, COLUMN-MAJOR, size nx*ny*nz)
// ---------------------------------------------------------------------------

// idx = i + nx*(j + ny*k), all 0-based.
inline std::size_t PIndex(int i, int j, int k, int nx, int ny) {
	return static_cast<std::size_t>(i) +
	       static_cast<std::size_t>(nx) *
	           (static_cast<std::size_t>(j) + static_cast<std::size_t>(ny) * k);
}

void InitP(int nx, int ny, int nz, const std::filesystem::path& path, bool p_ones = true);
void SaveP(const std::vector<double>& p_flat, const std::filesystem::path& path);
std::vector<double> LoadP(int nx, int ny, int nz, const std::filesystem::path& path);

// ValuesToIndx(boundaries[2], step) -> [lo, hi]   (1-based, inclusive; Julia semantics)
std::array<long, 2> ValuesToIndx(double b0, double b1, double step);

// De-duplicating concat, first occurrence wins (Julia `union`).
std::vector<std::string> unite(std::vector<std::string> a,
                               const std::vector<std::string>& b);

}  // namespace lucuma::julia
