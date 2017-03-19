Texture2D screenTex : register(t0);
SamplerState Sampler: register(s0);

struct PixelShaderInput
{
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};

float4 main(PixelShaderInput a) : SV_TARGET
{
	float4 color;
	color = screenTex.Sample(Sampler, a.texcoord);
	return color;
}