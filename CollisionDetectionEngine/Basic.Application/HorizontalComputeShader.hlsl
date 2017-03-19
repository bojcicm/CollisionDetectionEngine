Texture2D renderedSRV : register(t0);
RWTexture2D<float4> WritableUAV : register(u0);

#define size_x 1366
#define size_y 768 

static const float filter[7] = {
	0.030078323, 0.104983664, 0.222250419, 0.285375187, 0.222250419,
	0.104983664, 0.030078323
};

// Declare the group shared memory to hold the loaded and calculated data
groupshared float4 horizontalpointsEven[size_x/2][3];
groupshared float4 horizontalpointsOdd[size_x/2][3];

[numthreads(size_x/120, 1, 1)]
void main(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	// Load the current data from input texture
	float4 data = renderedSRV.Load(DispatchThreadID);

	// Stor the data into the GSM for the current thread
	if (DispatchThreadID.x % 2 != 0)
	{
		horizontalpointsOdd[DispatchThreadID.x][0] = data * filter[0];
		horizontalpointsOdd[DispatchThreadID.x][1] = data * filter[1];
		horizontalpointsOdd[DispatchThreadID.x][2] = data * filter[2];
	}
	else
	{
		horizontalpointsEven[DispatchThreadID.x][0] = data * filter[0];
		horizontalpointsEven[DispatchThreadID.x][1] = data * filter[1];
		horizontalpointsEven[DispatchThreadID.x][2] = data * filter[2];
	}
	// Synchronize all threads
	GroupMemoryBarrierWithGroupSync();
	
	// Offset the texture location to the first sample location
	int3 texturelocation = DispatchThreadID - int3(3, 0, 0);
	
	// Initialize the output value to zero, then loop through the
	// filter samples, apply them to the image samples, and sum
	// the results.
	float4 Color = float4(0.0, 9.0, 0.0, 0.0);
	if (texturelocation.x % 2 != 0)
	{
		Color += horizontalpointsOdd[texturelocation.x + 0][0];
		Color += horizontalpointsOdd[texturelocation.x + 1][1];
		Color += horizontalpointsOdd[texturelocation.x + 2][2];
		Color += data * filter[3];
		Color += horizontalpointsOdd[texturelocation.x + 4][2];
		Color += horizontalpointsOdd[texturelocation.x + 5][1];
		Color += horizontalpointsOdd[texturelocation.x + 6][0];
	}
	else
	{
		Color += horizontalpointsEven[texturelocation.x + 0][0];
		Color += horizontalpointsEven[texturelocation.x + 1][1];
		Color += horizontalpointsEven[texturelocation.x + 2][2];
		Color += data * filter[3];
		Color += horizontalpointsEven[texturelocation.x + 4][2];
		Color += horizontalpointsEven[texturelocation.x + 5][1];
		Color += horizontalpointsEven[texturelocation.x + 6][0];
	}

	// Write the output to the output resource
	WritableUAV[DispatchThreadID.xy] = Color;
}
