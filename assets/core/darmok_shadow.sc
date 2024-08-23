#ifndef DARMOK_SHADOW_HEADER
#define DARMOK_SHADOW_HEADER

#include <bgfx_shader.sh>

// vec2 texelSize
// uint cascadeAmount
uniform vec4 u_shadowData;

SAMPLER2DARRAYSHADOW(s_shadowMap, DARMOK_SAMPLER_SHADOW_MAP);
#define Sampler sampler2DShadowArray

// for each shadow map:
//   mat4 transform
BUFFER_RO(b_shadowTrans, vec4, DARMOK_SAMPLER_SHADOW_TRANS);

uint getShadowMapIndex(uint lightIndex, uint cascadeIndex)
{
	uint cascadeAmount = u_shadowData.z;
	return (lightIndex * cascadeAmount) + cascadeIndex;
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

float doHardShadow(uint shadowMapIndex, vec4 shadowCoord, float bias)
{
	vec3 texCoord = shadowCoord.xyz / shadowCoord.w;
	return shadow2DArray(s_shadowMap, vec4(texCoord.xy, shadowMapIndex, texCoord.z - bias));
}

float hardShadow(uint lightIndex, vec3 fragPos, float bias)
{
	uint cascadeAmount = u_shadowData.z;
	for(uint casc = 0; casc < cascadeAmount; ++casc)
	{
		uint mapIndex = getShadowMapIndex(lightIndex, casc);
		mat4 trans = getShadowTransform(mapIndex);
		vec4 shadowCoord = mul(trans, vec4(fragPos, 1.0));
		float v = doHardShadow(mapIndex, shadowCoord, bias);
		if(v < 1.0)
		{
			return v;
		}
	}
	return 1.0;
}

float normalShadowBias(vec3 norm, vec3 lightDir)
{
	return max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);  
}

// percentage closer filtering shadow
// https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf

float doPcfShadow(uint shadowMapIndex, vec4 shadowCoord, float bias)
{
	vec2 texCoord = shadowCoord.xy / shadowCoord.w;

	bool outside = any(greaterThan(texCoord, vec2_splat(1.0)))
				|| any(lessThan(texCoord, vec2_splat(0.0)))
				 ;

	if (outside)
	{
		return 1.0;
	}

	float result = 0.0;
	vec2 texelSize = u_shadowData.xy;
	vec2 offset = texelSize * shadowCoord.w;

	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(-1.5, -1.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(-1.5, -0.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(-1.5,  0.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(-1.5,  1.5) * offset, 0.0, 0.0), bias);

	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(-0.5, -1.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(-0.5, -0.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(-0.5,  0.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(-0.5,  1.5) * offset, 0.0, 0.0), bias);

	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(0.5, -1.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(0.5, -0.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(0.5,  0.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(0.5,  1.5) * offset, 0.0, 0.0), bias);

	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(1.5, -1.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(1.5, -0.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(1.5,  0.5) * offset, 0.0, 0.0), bias);
	result += doHardShadow(shadowMapIndex, shadowCoord + vec4(vec2(1.5,  1.5) * offset, 0.0, 0.0), bias);

	return result / 16.0;
}

float pcfShadow(uint lightIndex, vec3 fragPos, float bias)
{
	uint cascadeAmount = u_shadowData.z;
	float v = 0.0;
	for(uint casc = 0; casc < cascadeAmount; ++casc)
	{
		uint mapIndex = getShadowMapIndex(lightIndex, casc);
		mat4 trans = getShadowTransform(mapIndex);
		vec4 shadowCoord = mul(trans, vec4(fragPos, 1.0));
		v += doPcfShadow(mapIndex, shadowCoord, bias);
	}
	return v / cascadeAmount;
}

#endif // DARMOK_SHADOW_HEADER