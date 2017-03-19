Texture2D renderedSRV : register(t0);
RWTexture2D<float4> WritableUAV : register(u0);

#define size_x 32
#define size_y 32 

static const float filter[7][7] = {
	0.000904706, 0.003157733, 0.00668492, 0.008583607, 0.00668492,
	0.003157733, 0.000904706,
	0.003157733, 0.01102157, 0.023332663, 0.029959733, 0.023332663,
	0.01102157, 0.003157733,
	0.00668492, 0.023332663, 0.049395249, 0.063424755, 0.049395249,
	0.023332663, 0.00668492,
	0.008583607, 0.029959733, 0.063424755, 0.081438997, 0.063424755,
	0.029959733, 0.008583607,
	0.00668492, 0.023332663, 0.049395249, 0.063424755, 0.049395249,
	0.023332663, 0.00668492,
	0.003157733, 0.01102157, 0.023332663, 0.029959733, 0.023332663,
	0.01102157, 0.003157733,
	0.000904706, 0.003157733, 0.00668492, 0.008583607, 0.00668492,
	0.003157733, 0.000904706
};

// Declare the group shared memory to hold the loaded and calculated data
groupshared float4 horizontalpoints[size_x][3];

[numthreads(size_x, 1, 1)]
void main(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	
	// Offset the texture location to the first sample location
	int3 texturelocation = DispatchThreadID - int3(3, 3, 0);
		// Initializ e the output value to zero, then loop through the
		// filter samples, apply them to the image samples, and sum
		// the results .
		float4 Color = float4(0.0, 0.0, 0.0, 0.0);
		for (int x = 0; x < 7; x++)
			for(int y = 0; y < 7; y++)
				Color += renderedSRV.Load(texturelocation + int3(x, y, 0)) * filter[x][y];
	// Write the output to the output resource
	WritableUAV[DispatchThreadID.xy] = Color;
	
	//WritableUAV[DispatchThreadID.xy] = renderedSRV[DispatchThreadID.xy];
}
