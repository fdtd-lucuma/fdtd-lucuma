// == vis/funcs.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vis/funcs.hpp"

#include <charconv>
#include <cstdio>
#include <fstream>

namespace fs = std::filesystem;

namespace lucuma::julia {

void WriteMatrixCSV(const fs::path& path, const Mat2<double>& m) {
	if (path.has_parent_path()) fs::create_directories(path.parent_path());
	std::ofstream io(path);
	char buf[32];
	for (int i = 1; i <= m.rows(); ++i) {
		for (int j = 1; j <= m.cols(); ++j) {
			if (j > 1) io << ',';
			auto [p, ec] = std::to_chars(buf, buf + sizeof(buf), m(i, j));
			io.write(buf, p - buf);
		}
		io << '\n';
	}
}

void FDTDSnapshot(const Grid3<double>& field, const std::string& field_name,
                  int t, const std::string& additional, const fs::path& vis_path,
                  const std::string& plane) {
	fs::create_directories(vis_path);

	Mat2<double> data;
	std::string suffix;
	if (plane == "xz") {
		const int y_mid = field.ny() / 2;
		data = Mat2<double>(field.nx(), field.nz());
		for (int k = 1; k <= field.nz(); ++k)
			for (int i = 1; i <= field.nx(); ++i) data(i, k) = field(i, y_mid, k);
		suffix = "xz";
	} else if (plane == "yz") {
		const int x_mid = field.nx() / 2;
		data = Mat2<double>(field.ny(), field.nz());
		for (int k = 1; k <= field.nz(); ++k)
			for (int j = 1; j <= field.ny(); ++j) data(j, k) = field(x_mid, j, k);
		suffix = "yz";
	} else {
		const int z_mid = field.nz() / 2;
		data = Mat2<double>(field.nx(), field.ny());
		for (int j = 1; j <= field.ny(); ++j)
			for (int i = 1; i <= field.nx(); ++i) data(i, j) = field(i, j, z_mid);
		suffix = "xy";
	}

	char step[8];
	std::snprintf(step, sizeof(step), "%06d", t);
	WriteMatrixCSV(vis_path / (field_name + "_" + suffix + "_" + additional + "_" +
	                           step + ".csv"),
	               data);
}

}  // namespace lucuma::julia
