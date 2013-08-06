#include "simpleIncludeFile.inc"

shared extern float4 spriteRect;

uniform extern texture tex0;
uniform extern float4 spriteColor;

sampler texSampler = sampler_state
{
	Texture = <tex0>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU  = CLAMP;
    AddressV  = CLAMP;
};

struct OutputVS
{
	float4 posH : POSITION0;
	float2 texCoords : TEXCOORD0;
};
	
OutputVS TransformVS(float3 posL : POSITION0, float2 texCoords: TEXCOORD0)
{
	OutputVS outputV = (OutputVS)0;
	
	outputV.posH = float4(2.0f * (spriteRect.x + posL.x * spriteRect.z) / SCREEN_WIDTH - 1.0f, 
						 -(2.0f * (spriteRect.y + posL.y * spriteRect.w) / SCREEN_HEIGHT - 1.0f),
						 0, 1.0f);
	outputV.texCoords = texCoords;
	return outputV;
}

float4 TransformPS(float2 texCoords : TEXCOORD0) : COLOR
{
	float4 texColor = tex2D(texSampler, texCoords).rgba;
	return texColor * spriteColor;;
}

technique SpriteTechnique
{
	pass P0
	{
		vertexShader = compile vs_2_0 TransformVS();
		pixelShader = compile ps_2_0 TransformPS();	
	}
}