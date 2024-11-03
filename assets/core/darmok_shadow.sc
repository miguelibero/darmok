#ifndef DARMOK_SHADOW_HEADER
#define DARMOK_SHADOW_HEADER

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

// uint texelSize
// uint cascadeAmount
// float bias
// float normal bias
uniform vec4 u_shadowData;

#define u_shadowTexelSize		u_shadowData.x
#define u_shadowCascadeAmount	u_shadowData.y
#define u_shadowBias			u_shadowData.z
#define u_shadowNormalBias		u_shadowData.w

SAMPLER2DARRAYSHADOW(s_shadowMap, DARMOK_SAMPLER_SHADOW_MAP);
#define Sampler sampler2DShadowArray

// for each shadow map:
//   mat4 transform
BUFFER_RO(b_shadowTrans, vec4, DARMOK_SAMPLER_SHADOW_TRANS);

uint getShadowMapIndex(uint lightIndex, uint cascadeIndex)
{
	return (lightIndex * u_shadowCascadeAmount) + cascadeIndex;
}

mat4 getShadowTransform(uint shadowMapIndex)
{
	shadowMapIndex *= 4;
	return mat4(
		b_shadowTrans[shadowMapIndex + 0],
		b_shadowTrans[shadowMapIndex + 1],
		b_shadowTrans[shadowMapIndex + 2],
		b_shadowTrans[shadowMapIndex + 3]
	);
}

bool outsideShadowMap(vec3 texCoord)
{
	return any(greaterThan(texCoord, vec3_splat(1.0)))
		|| any(lessThan(texCoord, vec3_splat(0.0)))
	;
}

float getShadowMapValue(uint shadowMapIndex, vec3 texCoord)
{
	return shadow2DArray(s_shadowMap, vec4(texCoord.xy, shadowMapIndex, texCoord.z));
}

float normalShadowBias(vec3 norm, vec3 lightDir)
{
	return max(u_shadowNormalBias * (1.0 - dot(norm, lightDir)), u_shadowBias); 
}

float doHardShadow(uint lightIndex, vec3 fragPos, vec3 texOffset)
{
	for(uint casc = 0; casc < u_shadowCascadeAmount; ++casc)
	{
		uint mapIndex = getShadowMapIndex(lightIndex, casc);
		mat4 trans = getShadowTransform(mapIndex);
		vec4 shadowCoord = mul(trans, vec4(fragPos, 1.0));
		vec3 texCoord = shadowCoord.xyz / shadowCoord.w;
		texCoord += texOffset;
		if(!outsideShadowMap(texCoord))
		{
			return getShadowMapValue(mapIndex, texCoord);
		}
	}
	return 1.0;
}

float hardShadow(uint lightIndex, vec3 fragPos, float bias)
{
	return doHardShadow(lightIndex, fragPos, vec3(0.0, 0.0, -bias));
}

// percentage closer filtering shadow
// https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf

float softShadow(uint lightIndex, vec3 fragPos, float bias)
{
	float result = 0.0;
	float texelSize = u_shadowTexelSize;
	vec3 offset = vec3(texelSize, texelSize, 1.0);

	result += doHardShadow(lightIndex, fragPos, vec3(-1.5, -1.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(-1.5, -0.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(-1.5,  0.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(-1.5,  1.5, -bias) * offset);

	result += doHardShadow(lightIndex, fragPos, vec3(-0.5, -1.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(-0.5, -0.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(-0.5,  0.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(-0.5,  1.5, -bias) * offset);

	result += doHardShadow(lightIndex, fragPos, vec3(0.5, -1.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(0.5, -0.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(0.5,  0.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(0.5,  1.5, -bias) * offset);

	result += doHardShadow(lightIndex, fragPos, vec3(1.5, -1.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(1.5, -0.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(1.5,  0.5, -bias) * offset);
	result += doHardShadow(lightIndex, fragPos, vec3(1.5,  1.5, -bias) * offset);

	return result / 16.0;
}

#endif // DARMOK_SHADOW_HEADER