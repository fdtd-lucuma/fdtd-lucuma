#include "utils/types.slang"
#include "utils/matrix.slang"

struct PushConstants<T = FDTD_FLOAT_T>
{
	svec3 paddedDims;
	svec3 dims;
	T CrImp;
	T deltaT;
};

[[vk::constant_id(0)]] const uint wgX;
[[vk::constant_id(1)]] const uint wgY;
[[vk::constant_id(2)]] const uint wgZ;

RWStructuredBuffer<FDTD_FLOAT_T> _Ch;
RWStructuredBuffer<FDTD_FLOAT_T> _Ce;
StructuredBuffer<FDTD_FLOAT_T> _CM;
StructuredBuffer<FDTD_FLOAT_T> _mu;

bool inBoundaries(uint3 threadId, svec3 dims)
{
	return threadId.x < dims.x && threadId.y < dims.y && threadId.z < dims.z;
}

[shader("compute")]
[numthreads(wgX,wgY,wgZ)]
void main(uint3 threadId : SV_DispatchThreadID, uniform PushConstants pushConstants)
{
	if(!inBoundaries(threadId, pushConstants.dims))
		return;

	matrix3d Ch = matrix3d<FDTD_FLOAT_T>(_Ch, pushConstants.paddedDims);
	matrix3d Ce = matrix3d<FDTD_FLOAT_T>(_Ce, pushConstants.paddedDims);

	cmatrix3d CM = cmatrix3d<FDTD_FLOAT_T>(_CM, pushConstants.paddedDims);
	cmatrix3d mu = cmatrix3d<FDTD_FLOAT_T>(_mu, pushConstants.paddedDims);

	uint i = threadId.x;
	uint j = threadId.y;
	uint k = threadId.z;

	const FDTD_FLOAT_T c = (CM[i,j,k]*pushConstants.deltaT)/(2*mu[i,j,k]);

	Ch[i,j,k] = (1-c)/(1+c);
	Ce[i,j,k] = (1/(1+c))*pushConstants.CrImp;

}
