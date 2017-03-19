struct PixelShaderInput
{
	float4 pos : SV_Position;
	float4 color: COLOR;
};

float4 main(PixelShaderInput a) : SV_TARGET
{
	float4 color;
	color = float4(a.color.r, a.color.g, a.color.b, a.pos.z);
	return color;
}