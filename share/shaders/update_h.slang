#include "utils/types.slang"
#include "utils/matrix.slang"
#include "utils/workgroup.slang"

struct PushConstants<T = FDTD_FLOAT_T>
{
	svec3 paddedHDims;
	svec3 paddedEc1Dims;
	svec3 paddedEc2Dims;
	svec3 Hdims;
	T deltaT;
	T delta1;
	T delta2;
};

[[vk::constant_id(0)]] const uint wgX;
[[vk::constant_id(1)]] const uint wgY;
[[vk::constant_id(2)]] const uint wgZ;

[[vk::constant_id(3)]] const int Ec1DeltaX;
[[vk::constant_id(4)]] const int Ec1DeltaY;
[[vk::constant_id(5)]] const int Ec1DeltaZ;

[[vk::constant_id(6)]] const int Ec2DeltaX;
[[vk::constant_id(7)]] const int Ec2DeltaY;
[[vk::constant_id(8)]] const int Ec2DeltaZ;

RWStructuredBuffer<FDTD_FLOAT_T> _Hc;
StructuredBuffer<FDTD_FLOAT_T> _Ch;
StructuredBuffer<FDTD_FLOAT_T> _Ce;
StructuredBuffer<FDTD_FLOAT_T> _Ec1;
StructuredBuffer<FDTD_FLOAT_T> _Ec2;

bool outside(uint3 threadId, svec3 dims)
{
	return any(threadId >= dims);
}

[shader("compute")]
Workgroup(wgX,wgY,wgZ)
void main(uint3 threadId : SV_DispatchThreadID, uniform PushConstants pushConstants)
{
	if(outside(threadId, pushConstants.Hdims))
		return;

	matrix3d Hc = matrix3d<FDTD_FLOAT_T>(_Hc, pushConstants.paddedHDims);

	cmatrix3d Ch = cmatrix3d<FDTD_FLOAT_T>(_Ch, pushConstants.paddedHDims);
	cmatrix3d Ce = cmatrix3d<FDTD_FLOAT_T>(_Ce, pushConstants.paddedHDims);

	cmatrix3d Ec1 = cmatrix3d<FDTD_FLOAT_T>(_Ec1, pushConstants.paddedEc1Dims);
	cmatrix3d Ec2 = cmatrix3d<FDTD_FLOAT_T>(_Ec2, pushConstants.paddedEc2Dims);

	const uint i = threadId.x;
	const uint j = threadId.y;
	const uint k = threadId.z;

	const uint Ec1i = i + Ec1DeltaX;
	const uint Ec1j = j + Ec1DeltaY;
	const uint Ec1k = k + Ec1DeltaZ;

	const uint Ec2i = i + Ec2DeltaX;
	const uint Ec2j = j + Ec2DeltaY;
	const uint Ec2k = k + Ec2DeltaZ;

	Hc[i,j,k] = Ch[i,j,k]*Hc[i,j,k] + Ce[i,j,k] *
		pushConstants.deltaT*((Ec1[Ec1i,Ec1j,Ec1k]-Ec1[i,j,k])/pushConstants.delta2 - (Ec2[Ec2i,Ec2j,Ec2k]-Ec2[i,j,k])/pushConstants.delta1)
	;
}
