#include "utils/types.slang"
#include "utils/matrix.slang"

struct PushConstants<T = FDTD_FLOAT_T>
{
	svec3 paddedEDims;
	svec3 paddedHc1Dims;
	svec3 paddedHc2Dims;
	svec3 dims;
	svec3 start;
	T deltaT;
	T delta1;
	T delta2;
};

[[vk::constant_id(0)]] const uint wgX;
[[vk::constant_id(1)]] const uint wgY;
[[vk::constant_id(2)]] const uint wgZ;

[[vk::constant_id(3)]] const int Hc1DeltaX;
[[vk::constant_id(4)]] const int Hc1DeltaY;
[[vk::constant_id(5)]] const int Hc1DeltaZ;

[[vk::constant_id(6)]] const int Hc2DeltaX;
[[vk::constant_id(7)]] const int Hc2DeltaY;
[[vk::constant_id(8)]] const int Hc2DeltaZ;

RWStructuredBuffer<FDTD_FLOAT_T> _Ec;
StructuredBuffer<FDTD_FLOAT_T> _Ce;
StructuredBuffer<FDTD_FLOAT_T> _Ch;
StructuredBuffer<FDTD_FLOAT_T> _Hc1;
StructuredBuffer<FDTD_FLOAT_T> _Hc2;

bool outside(uint3 threadId, svec3 dims, svec3 start)
{
	return any(threadId >= dims) || any(threadId < start);
}

[shader("compute")]
[numthreads(wgX,wgY,wgZ)]
void main(uint3 threadId : SV_DispatchThreadID, uniform PushConstants pushConstants)
{
	if(outside(threadId, pushConstants.dims, pushConstants.start))
		return;

	matrix3d Ec = matrix3d<FDTD_FLOAT_T>(_Ec, pushConstants.paddedEDims);

	cmatrix3d Ce = cmatrix3d<FDTD_FLOAT_T>(_Ce, pushConstants.paddedEDims);
	cmatrix3d Ch = cmatrix3d<FDTD_FLOAT_T>(_Ch, pushConstants.paddedEDims);

	cmatrix3d Hc1 = cmatrix3d<FDTD_FLOAT_T>(_Hc1, pushConstants.paddedHc1Dims);
	cmatrix3d Hc2 = cmatrix3d<FDTD_FLOAT_T>(_Hc2, pushConstants.paddedHc2Dims);

	const uint i = threadId.x;
	const uint j = threadId.y;
	const uint k = threadId.z;

	const uint Hc1i = i + Hc1DeltaX;
	const uint Hc1j = j + Hc1DeltaY;
	const uint Hc1k = k + Hc1DeltaZ;

	const uint Hc2i = i + Hc2DeltaX;
	const uint Hc2j = j + Hc2DeltaY;
	const uint Hc2k = k + Hc2DeltaZ;


	Ec[i,j,k] = Ce[i,j,k]*Ec[i,j,k] + Ch[i,j,k] *
		pushConstants.deltaT*((Hc1[i,j,k]-Hc1[Hc1i,Hc1j,Hc1k])/pushConstants.delta2 - (Hc2[i,j,k]-Hc2[Hc2i,Hc2j,Hc2k])/pushConstants.delta1)
	;
}
