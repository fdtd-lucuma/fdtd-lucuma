#include "utils/types.slang"
#include "utils/matrix.slang"

[[vk::constant_id(0)]] const uint wgX;
[[vk::constant_id(1)]] const uint wgY;
[[vk::constant_id(2)]] const uint wgZ;

RWStructuredBuffer<FDTD_FLOAT_T> Ch;
RWStructuredBuffer<FDTD_FLOAT_T> Ce;
StructuredBuffer<FDTD_FLOAT_T> CM;
StructuredBuffer<FDTD_FLOAT_T> mu;

[shader("compute")]
[numthreads(wgX,wgY,wgZ)]
void main(uint3 threadId : SV_DispatchThreadID)
{
}
