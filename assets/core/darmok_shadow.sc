#ifndef DARMOK_SHADOW_HEADER
#define DARMOK_SHADOW_HEADER

#include <bgfx_shader.sh>

uniform mat4 u_dirLightTrans;
uniform vec4 u_shadowMapData;

SAMPLER2DSHADOW(s_shadowMap, DARMOK_SAMPLER_SHADOW_MAP);
#define Sampler sampler2DShadow

float hardShadow(Sampler shadowMap, vec4 shadowCoord, float bias)
{
	vec3 texCoord = shadowCoord.xyz / shadowCoord.w;
	return shadow2D(shadowMap, vec3(texCoord.xy, texCoord.z - bias));
}

// percentage closer filtering shadow
// https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf
float PCF(Sampler shadowMap, vec4 shadowCoord, float bias, vec2 texelSize)
{
	vec2 texCoord = shadowCoord.xy / shadowCoord.w;

	bool outside = any(greaterThan(texCoord, vec2_splat(1.0)))
				|| any(lessThan   (texCoord, vec2_splat(0.0)))
				 ;

	if (outside)
	{
		return 1.0;
	}

	float result = 0.0;
	vec2 offset = texelSize * shadowCoord.w;

	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(-1.5, -1.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(-1.5, -0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(-1.5,  0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(-1.5,  1.5) * offset, 0.0, 0.0), bias);

	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(-0.5, -1.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(-0.5, -0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(-0.5,  0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(-0.5,  1.5) * offset, 0.0, 0.0), bias);

	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(0.5, -1.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(0.5, -0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(0.5,  0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(0.5,  1.5) * offset, 0.0, 0.0), bias);

	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(1.5, -1.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(1.5, -0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(1.5,  0.5) * offset, 0.0, 0.0), bias);
	result += hardShadow(shadowMap, shadowCoord + vec4(vec2(1.5,  1.5) * offset, 0.0, 0.0), bias);

	return result / 16.0;
}

#endif // DARMOK_SHADOW_HEADER