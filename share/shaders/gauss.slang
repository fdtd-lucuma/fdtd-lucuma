#include "utils/types.slang"
#include "utils/matrix.slang"

struct PushConstants<T = FDTD_FLOAT_T>
{
	svec3 paddedDims;
	svec3 pos;
	T time;
	T sigma;
	T x0;
};

RWStructuredBuffer<FDTD_FLOAT_T> _Ec;

// How do slang generics work?
FDTD_FLOAT_T gauss(FDTD_FLOAT_T time, FDTD_FLOAT_T sigma, FDTD_FLOAT_T x0)
{
	const FDTD_FLOAT_T x = (time-x0)/sigma;

	return exp<FDTD_FLOAT_T>(-(x*x));
}

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID, uniform PushConstants pushConstants)
{
	matrix3d Ec = matrix3d<FDTD_FLOAT_T>(_Ec, pushConstants.paddedDims);

	const uint i = (uint)pushConstants.pos.x;
	const uint j = (uint)pushConstants.pos.y;
	const uint k = (uint)pushConstants.pos.z;

	Ec[i,j,k] += gauss(pushConstants.time, pushConstants.sigma, pushConstants.x0);

}
