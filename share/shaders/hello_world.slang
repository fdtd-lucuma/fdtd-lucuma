#include "utils/types.slang"
#include "utils/matrix.slang"

[[vk::constant_id(0)]] const uint wgX;
[[vk::constant_id(1)]] const uint wgY;
[[vk::constant_id(2)]] const uint wgZ;

[shader("compute")]
[numthreads(wgX,wgY,wgZ)]
void main(
	uint3 threadId : SV_DispatchThreadID,
	StructuredBuffer<FDTD_FLOAT_T> a,
	StructuredBuffer<FDTD_FLOAT_T> b,
	RWStructuredBuffer<FDTD_FLOAT_T> c,
)
{
	uint index = threadId.x;
	c[index] = a[index] + b[index];
}
