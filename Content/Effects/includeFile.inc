shared float4x4 view;
shared float4x4 projection;
shared float4 lightColor;
shared float4 lightColor2;
shared float4 ambient;

shared float3 camPos;
shared float specularPower;
shared float3 specularColor;

shared float3 lightPos;
shared float3 lightPos2;

shared float4 fogColor;
shared bool fogEnabled;

float4 calcPhong(float3 worldPos, float3 wNormal, float4 diffuseColor) 
{

	float4 outColor = ambient * diffuseColor;
	float3 lightDir = normalize(lightPos - worldPos);
	float3 lightDir2 = normalize(lightPos2 - worldPos);
	float d = saturate(dot(lightDir, wNormal));
	float d2 = saturate(dot(lightDir2, wNormal));
	outColor += diffuseColor * lightColor * d;
	outColor += diffuseColor * lightColor2 * d2;

	float3 viewDir = normalize(camPos-worldPos);
	
	float3 refl = normalize(reflect(-lightDir, wNormal));
	float3 refl2 = normalize(reflect(-lightDir2, wNormal));
	outColor += pow(saturate(dot(refl, viewDir)), specularPower) * float4(specularColor, 1.0) 
				* lightColor;// * diffuseColor;
	outColor += pow(saturate(dot(refl2, viewDir)), specularPower) * float4(specularColor, 1.0) 
				* lightColor;// * diffuseColor;

	outColor.a = diffuseColor.a;
	
	if (fogEnabled) {
		float dist=length(worldPos-camPos);
		float l = saturate((dist-1500)/(3000-1500));
		outColor=lerp(outColor, fogColor, l);
	}

	return outColor;
}


// for particle systms
float getSize(float baseSize, float viewHeight, float4 projectedPos) {

    return baseSize * projection._m11 / projectedPos.w * viewHeight / 2;
}