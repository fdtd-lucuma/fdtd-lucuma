#pragma once

#include "fdtd_macros.slang"

struct cmatrix3d<T = FDTD_FLOAT_T>
{
	typealias buffer_t = StructuredBuffer<T>;

	buffer_t buffer;
	int x;
	int y;
	int z;

	__init(buffer_t _buffer, int _x, int _y, int _z)
	{
		buffer = _buffer;
		x = _x;
		y = _y;
		z = _z;
	}

	__subscript(int i, int j, int k) -> T
	{
		get { return buffer[i*(y*z)+j*z+k]; }
	}
};

// TODO: Find a way to dedup this
struct matrix3d<T = FDTD_FLOAT_T>
{
	typealias buffer_t = RWStructuredBuffer<T>;

	buffer_t buffer;
	int x;
	int y;
	int z;

	__init(buffer_t _buffer, int _x, int _y, int _z)
	{
		buffer = _buffer;
		x = _x;
		y = _y;
		z = _z;
	}

	__subscript(int i, int j, int k) -> T
	{
		get { return buffer[i*(y*z)+j*z+k]; }
		set { buffer[i*(y*z)+j*z+k] = newValue; }
	}
};

struct cmatrix2d<T = FDTD_FLOAT_T>
{
	typealias buffer_t = StructuredBuffer<T>;

	buffer_t buffer;
	int x;
	int y;

	__init(buffer_t _buffer, int _x, int _y)
	{
		buffer = _buffer;
		x = _x;
		y = _y;
	}

	__subscript(int i, int j, int k) -> T
	{
		get { return buffer[i*y+j]; }
	}
};

// TODO: Find a way to dedup this
struct matrix2d<T = FDTD_FLOAT_T>
{
	typealias buffer_t = RWStructuredBuffer<T>;

	buffer_t buffer;
	int x;
	int y;

	__init(buffer_t _buffer, int _x, int _y)
	{
		buffer = _buffer;
		x = _x;
		y = _y;
	}

	__subscript(int i, int j) -> T
	{
		get { return buffer[i*y+j]; }
		set { buffer[i*y+j] = newValue; }
	}
};
