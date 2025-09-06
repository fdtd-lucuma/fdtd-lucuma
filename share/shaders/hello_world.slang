StructuredBuffer<float> a;
StructuredBuffer<float> b;
RWStructuredBuffer<float> c;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
	uint index = threadId.x;
	c[index] = a[index] + b[index];
}
