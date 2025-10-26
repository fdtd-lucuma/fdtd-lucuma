#include "utils/types.slang"
#include "utils/matrix.slang"

struct PushConstants<T = FDTD_FLOAT_T>
{
	svec3 paddedDims;
	svec3 paddedEcDims;
	svec3 dims;
	svec2 paddedecDims;
	T Cr;
};

[[vk::constant_id(0)]] const uint wgX;
[[vk::constant_id(1)]] const uint wgY;
[[vk::constant_id(2)]] const uint wgZ;

[[vk::constant_id(3)]] const uint _dim;
[[vk::constant_id(4)]] const int sliceIndex;
[[vk::constant_id(5)]] const int sliceIndex2;

StructuredBuffer<FDTD_FLOAT_T> _mu;
StructuredBuffer<FDTD_FLOAT_T> _eps;
RWStructuredBuffer<FDTD_FLOAT_T> _Ec;
RWStructuredBuffer<FDTD_FLOAT_T> _ec;

bool outside(uint3 threadId, svec3 dims)
{
	return any(threadId >= dims);
}

uint getI(Dim dim, uint3 index)
{
	switch(dim)
	{
		case Dim::X:
			return index.y;
		case Dim::Y:
		case Dim::Z:
		default:
			return index.x;
	}
}

uint getJ(Dim dim, uint3 index)
{
	switch(dim)
	{
		case Dim::X:
		case Dim::Y:
			return index.z;
		case Dim::Z:
		default:
			return index.y;
	}
}

[shader("compute")]
[numthreads(wgX,wgY,wgZ)]
void main(uint3 threadId : SV_DispatchThreadID, uniform PushConstants pushConstants)
{
	const Dim dim = (Dim)_dim;

	if(outside(threadId, pushConstants.paddedEcDims))
		return;

	cmatrix3d __mu  = cmatrix3d<FDTD_FLOAT_T>(_mu, pushConstants.paddedDims);
	cmatrix3d __eps = cmatrix3d<FDTD_FLOAT_T>(_eps, pushConstants.paddedDims);

	matrix3d __Ec = matrix3d<FDTD_FLOAT_T>(_Ec, pushConstants.paddedEcDims);
	matrix2d ec   = matrix2d<FDTD_FLOAT_T>(_ec, pushConstants.paddedecDims);

	slicedCmatrix2d mu  = slicedCmatrix2d<FDTD_FLOAT_T>(__mu, dim, sliceIndex);
	slicedCmatrix2d eps = slicedCmatrix2d<FDTD_FLOAT_T>(__eps, dim, sliceIndex);
	slicedMatrix2d Ec   = slicedMatrix2d<FDTD_FLOAT_T>(__Ec, dim, sliceIndex);
	slicedMatrix2d Ecd  = slicedMatrix2d<FDTD_FLOAT_T>(__Ec, dim, sliceIndex2);

	const uint i = getI(dim, threadId);
	const uint j = getJ(dim, threadId);

	const FDTD_FLOAT_T Sc = pushConstants.Cr/sqrt<FDTD_FLOAT_T>(mu[i,j]*eps[i,j]);
	const FDTD_FLOAT_T abcCoef = (Sc-1)/(Sc+1);

	Ec[i,j] = ec[i,j] + abcCoef*(Ecd[i,j]-Ec[i,j]);
	ec[i,j] = Ecd[i,j];
}
