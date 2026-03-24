#include "utils/types.slang"
#include "utils/matrix.slang"

struct PushConstants<T = FDTD_FLOAT_T>
{
	svec3 paddedDims;
	T time;
	T x0;
};

struct SourceSsbo<T = FDTD_FLOAT_T>
{
	svec3 position;
	T sigma;
};

[[vk::constant_id(0)]] const uint wgX;
[[vk::constant_id(1)]] const uint wgY;
[[vk::constant_id(2)]] const uint wgZ;
[[vk::constant_id(3)]] const uint size;

RWStructuredBuffer<FDTD_FLOAT_T> _Ec;
StructuredBuffer<SourceSsbo> sources;

// How do slang generics work?
FDTD_FLOAT_T gauss(FDTD_FLOAT_T time, FDTD_FLOAT_T sigma, FDTD_FLOAT_T x0)
{
	const FDTD_FLOAT_T x = (time-x0)/sigma;

	return exp<FDTD_FLOAT_T>(-(x*x));
}

[shader("compute")]
[numthreads(wgX,wgY,wgZ)]
void main(uint3 threadId : SV_DispatchThreadID, uniform PushConstants pushConstants)
{
	if(threadId.x > size)
		return;

	matrix3d Ec = matrix3d<FDTD_FLOAT_T>(_Ec, pushConstants.paddedDims);

	SourceSsbo source = sources[threadId.x];

	const uint i = (uint)source.position.x;
	const uint j = (uint)source.position.y;
	const uint k = (uint)source.position.z;

	Ec[i,j,k] += gauss(pushConstants.time, source.sigma, pushConstants.x0);

}
