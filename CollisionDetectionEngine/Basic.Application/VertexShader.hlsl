cbuffer WorldTransforms : register(b0)
{
	matrix World;
	matrix Dlrow;
};
cbuffer ViewTransforms : register(b1)
{
	matrix View;
};
cbuffer ProjectionTransforms : register(b2)
{
	matrix Projection;
};

struct VertexShaderInput
{
	float3 pos : SV_Position;
	vector<float, 4> color : COLOR;
};
struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput a)
{
	float4 pos = float4(a.pos, 1.0f);
	pos = mul(pos, World);
	pos = mul(pos, View);
	pos = mul(pos, Projection);
	VertexShaderOutput b;
	b.pos = pos;
	b.color = a.color;
	return b;
}