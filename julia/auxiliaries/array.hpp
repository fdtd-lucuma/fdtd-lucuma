// No direct Julia counterpart — Julia uses builtin column-major Arrays.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Minimal column-major, 1-BASED dense containers so the ported kernels can be
// transcribed index-for-index from the Julia / CUDA source (e.g. `Ey(i,j,k+1)`).

#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace lucuma::vulkan {

template <class T>
class Mat2 {
public:
	Mat2() = default;
	Mat2(int rows, int cols, T v = T{})
	    : rows_(rows), cols_(cols),
	      data_(static_cast<std::size_t>(rows) * cols, v) {}

	// 1-based, column-major: idx = (i-1) + rows*(j-1)
	T& operator()(int i, int j) {
		return data_[static_cast<std::size_t>(i - 1) +
		             static_cast<std::size_t>(rows_) * (j - 1)];
	}
	const T& operator()(int i, int j) const {
		return data_[static_cast<std::size_t>(i - 1) +
		             static_cast<std::size_t>(rows_) * (j - 1)];
	}

	int rows() const { return rows_; }
	int cols() const { return cols_; }
	std::size_t size() const { return data_.size(); }
	T* data() { return data_.data(); }
	const T* data() const { return data_.data(); }
	void fill(T v) { std::fill(data_.begin(), data_.end(), v); }

private:
	int rows_ = 0, cols_ = 0;
	std::vector<T> data_;
};

template <class T>
class Grid3 {
public:
	Grid3() = default;
	Grid3(int nx, int ny, int nz, T v = T{})
	    : nx_(nx), ny_(ny), nz_(nz),
	      data_(static_cast<std::size_t>(nx) * ny * nz, v) {}

	// 1-based, column-major: idx = (i-1) + nx*((j-1) + ny*(k-1))
	T& operator()(int i, int j, int k) { return data_[idx(i, j, k)]; }
	const T& operator()(int i, int j, int k) const { return data_[idx(i, j, k)]; }

	int nx() const { return nx_; }
	int ny() const { return ny_; }
	int nz() const { return nz_; }
	std::size_t size() const { return data_.size(); }
	T* data() { return data_.data(); }
	const T* data() const { return data_.data(); }
	void fill(T v) { std::fill(data_.begin(), data_.end(), v); }

private:
	std::size_t idx(int i, int j, int k) const {
		return static_cast<std::size_t>(i - 1) +
		       static_cast<std::size_t>(nx_) *
		           (static_cast<std::size_t>(j - 1) +
		            static_cast<std::size_t>(ny_) * (k - 1));
	}
	int nx_ = 0, ny_ = 0, nz_ = 0;
	std::vector<T> data_;
};

}  // namespace lucuma::vulkan
