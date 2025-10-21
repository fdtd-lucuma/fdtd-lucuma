#pragma once

#include "types.slang"
#include "enums.slang"

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

	__init(buffer_t _buffer, svec3 dims)
	{
		buffer = _buffer;
		x = (uint)dims.x;
		y = (uint)dims.y;
		z = (uint)dims.z;
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

	__init(buffer_t _buffer, svec3 dims)
	{
		buffer = _buffer;
		x = (uint)dims.x;
		y = (uint)dims.y;
		z = (uint)dims.z;
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

	__init(buffer_t _buffer, svec2 dims)
	{
		buffer = _buffer;
		x = (uint)dims.x;
		y = (uint)dims.y;
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

	__init(buffer_t _buffer, svec2 dims)
	{
		buffer = _buffer;
		x = (uint)dims.x;
		y = (uint)dims.y;
	}

	__subscript(uint i, uint j) -> T
	{
		get { return buffer[i*y+j]; }
		set { buffer[i*y+j] = newValue; }
	}
};

struct slicedCmatrix2d<T = FDTD_FLOAT_T>
{
	typealias matrix_t = cmatrix3d<T>;

	matrix_t matrix;
	Dim dim;
	uint s;

	__init(matrix_t _matrix, Dim _dim, uint _s)
	{
		matrix = _matrix;
		dim = _dim;
		s = _s;
	}

	__subscript(uint i, uint j) -> T
	{
		get { return matrix[getI(i,j), getJ(i,j), getK(i,j)]; }
	}

	uint getI(uint i, uint j)
	{
		switch(dim)
		{
			case Dim::X:
				return s;
			case Dim::Y:
			case Dim::Z:
				return i;
		}
	}

	uint getJ(uint i, uint j)
	{
		switch(dim)
		{
			case Dim::X:
				return i;
			case Dim::Y:
				return s;
			case Dim::Z:
				return j;
		}
	}

	uint getK(uint i, uint j)
	{
		switch(dim)
		{
			case Dim::X:
			case Dim::Y:
				return j;
			case Dim::Z:
				return s;
		}
	}
};

// TODO: Find a way to dedup this
struct slicedMatrix2d<T = FDTD_FLOAT_T>
{
	typealias matrix_t = matrix3d<T>;

	matrix_t matrix;
	Dim dim;
	uint s;

	__init(matrix_t _matrix, Dim _dim, uint _s)
	{
		matrix = _matrix;
		dim = _dim;
		s = _s;
	}

	__subscript(uint i, uint j) -> T
	{
		get { return matrix[getI(i,j), getJ(i,j), getK(i,j)]; }
		set { matrix[getI(i,j), getJ(i,j), getK(i,j)] = newValue; }
	}

	uint getI(uint i, uint j)
	{
		switch(dim)
		{
			case Dim::X:
				return s;
			case Dim::Y:
			case Dim::Z:
				return i;
		}
	}

	uint getJ(uint i, uint j)
	{
		switch(dim)
		{
			case Dim::X:
				return i;
			case Dim::Y:
				return s;
			case Dim::Z:
				return j;
		}
	}

	uint getK(uint i, uint j)
	{
		switch(dim)
		{
			case Dim::X:
			case Dim::Y:
				return j;
			case Dim::Z:
				return s;
		}
	}
};
