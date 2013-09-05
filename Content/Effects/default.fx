float4x4 world;
float3 diffuseColor;

#include "includeFile.inc"

struct VSOutput
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : TEXCOORD1;
	float3 PositionW : TEXCOORD2;
};

struct PSInput {
	float2 TexCoord : TEXCOORD0;
	float3 Normal : TEXCOORD1;
	float3 PositionW : TEXCOORD2;
};

texture Texture;

sampler2D TextureSampler = sampler_state
{
	texture = <Texture>;
	magFilter = LINEAR;
	minFilter = LINEAR;
	mipFilter = LINEAR;
	AddressU = Mirror;
	AddressV = Mirror;
};

VSOutput BasicVS(float3 position : POSITION0, float3 normal : NORMAL0,
				float2 TexCoord : TEXCOORD0) 
{
	VSOutput output;
	float4x4 trans = mul(mul(world, view), projection);
	output.Position = mul(float4(position, 1.0f), trans);

	output.PositionW = mul(float4(position, 1.0f), world).xyz;

	output.Normal = mul(float4(normal, 0.0), world).xyz;
	output.TexCoord = TexCoord;
	output.TexCoord.y = 1+output.TexCoord.y;
	return output;
}
	
float4 BasicPS(PSInput input) : COLOR
{
	float4 baseColor;

	baseColor = tex2D(TextureSampler, input.TexCoord) * float4(diffuseColor, 1.0);
			
	float4 outColor = calcPhong(input.PositionW, normalize(input.Normal), baseColor);
	
	outColor = outColor + ambient;
	outColor.a = 1;
	return outColor;
}

technique Phong
{
	pass pass1
	{
		VertexShader = compile vs_2_0 BasicVS();
		PixelShader = compile ps_2_0 BasicPS();
	}
}
