#pragma once

#include "types.slang"

struct cmatrix3d<T = FDTD_FLOAT_T>
{
	typealias buffer_t = StructuredBuffer<T>;

	buffer_t buffer;
	uint x;
	uint y;
	uint z;

	__init(buffer_t _buffer, uint _x, uint _y, uint _z)
	{
		buffer = _buffer;
		x = _x;
		y = _y;
		z = _z;
	}

	__subscript(uint i, uint j, uint k) -> T
	{
		get { return buffer[i*(y*z)+j*z+k]; }
	}
};

// TODO: Find a way to dedup this
struct matrix3d<T = FDTD_FLOAT_T>
{
	typealias buffer_t = RWStructuredBuffer<T>;

	buffer_t buffer;
	uint x;
	uint y;
	uint z;

	__init(buffer_t _buffer, uint _x, uint _y, uint _z)
	{
		buffer = _buffer;
		x = _x;
		y = _y;
		z = _z;
	}

	__subscript(uint i, uint j, uint k) -> T
	{
		get { return buffer[i*(y*z)+j*z+k]; }
		set { buffer[i*(y*z)+j*z+k] = newValue; }
	}
};

struct cmatrix2d<T = FDTD_FLOAT_T>
{
	typealias buffer_t = StructuredBuffer<T>;

	buffer_t buffer;
	uint x;
	uint y;

	__init(buffer_t _buffer, uint _x, uint _y)
	{
		buffer = _buffer;
		x = _x;
		y = _y;
	}

	__subscript(uint i, uint j, uint k) -> T
	{
		get { return buffer[i*y+j]; }
	}
};

// TODO: Find a way to dedup this
struct matrix2d<T = FDTD_FLOAT_T>
{
	typealias buffer_t = RWStructuredBuffer<T>;

	buffer_t buffer;
	uint x;
	uint y;

	__init(buffer_t _buffer, uint _x, uint _y)
	{
		buffer = _buffer;
		x = _x;
		y = _y;
	}

	__subscript(uint i, uint j) -> T
	{
		get { return buffer[i*y+j]; }
		set { buffer[i*y+j] = newValue; }
	}
};
