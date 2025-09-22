#ifndef FDTD_FLOAT_T
#define FDTD_FLOAT_T float
#endif

[shader("compute")]
[numthreads(1,1,1)]
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
