#ifndef DARMOK_SHADOW_HEADER
#define DARMOK_SHADOW_HEADER

#include <bgfx_shader.sh>

uniform vec4 u_shadowData;

SAMPLER2DARRAYSHADOW(s_shadowMap, DARMOK_SAMPLER_SHADOW_MAP);
#define Sampler sampler2DShadowArray

// for each shadow map:
//   mat4 transform
BUFFER_RO(b_shadowTrans, vec4, DARMOK_SAMPLER_SHADOW_TRANS);

mat4 getShadowTransform(uint lightIndex)
{
	lightIndex *= 4;
	return mat4(
		b_shadowTrans[lightIndex + 0],
		b_shadowTrans[lightIndex + 1],
		b_shadowTrans[lightIndex + 2],
		b_shadowTrans[lightIndex + 3]
	);
}

float hardShadow(uint lightIndex, vec4 shadowCoord, float bias)
{
	vec3 texCoord = shadowCoord.xyz / shadowCoord.w;
	return shadow2DArray(s_shadowMap, vec4(texCoord.xy, lightIndex, texCoord.z - bias));
}

// percentage closer filtering shadow
// https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf

float PCF(uint lightIndex, vec3 fragPos, float bias)
{
	mat4 trans = getShadowTransform(lightIndex);
	vec4 shadowCoord = mul(trans, vec4(fragPos, 1.0));
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

	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(-1.5, -1.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(-1.5, -0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(-1.5,  0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(-1.5,  1.5) * offset, 0.0, 0.0), bias);

	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(-0.5, -1.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(-0.5, -0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(-0.5,  0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(-0.5,  1.5) * offset, 0.0, 0.0), bias);

	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(0.5, -1.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(0.5, -0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(0.5,  0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(0.5,  1.5) * offset, 0.0, 0.0), bias);

	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(1.5, -1.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(1.5, -0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(1.5,  0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(lightIndex, shadowCoord + vec4(vec2(1.5,  1.5) * offset, 0.0, 0.0), bias);

	return result / 16.0;
}

#endif // DARMOK_SHADOW_HEADER