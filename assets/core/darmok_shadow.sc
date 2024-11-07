#ifndef DARMOK_SHADOW_HEADER
#define DARMOK_SHADOW_HEADER

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include <darmok_light.sc>

uniform vec4 u_shadowData1;
#define u_shadowTexelSize		u_shadowData1.x
#define u_shadowCascadeAmount	uint(u_shadowData1.y)
#define u_shadowBias			u_shadowData1.z
#define u_shadowNormalBias		u_shadowData1.w

uniform vec4 u_shadowData2;
#define u_shadowDirAmout		uint(u_shadowData2.x)
#define u_shadowSpotAmount		uint(u_shadowData2.y)
#define u_shadowPointAmount		uint(u_shadowData2.z)
#define u_shadowSoftMask		uint(u_shadowData2.w)

// light shadow maps ordered by type: directional (cascaded), spot, point
SAMPLER2DARRAYSHADOW(s_shadowMap, DARMOK_SAMPLER_SHADOW_MAP);
#define Sampler sampler2DShadowArray

// for each shadow map:
//   mat4 transform
BUFFER_RO(b_shadowTrans, vec4, DARMOK_SAMPLER_SHADOW_TRANS);

// for each directional light:
//   vec4 direction
BUFFER_RO(b_shadowDirection, vec4, DARMOK_SAMPLER_SHADOW_DIR);

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

float doHardShadow(uint startIdx, uint countIdx, vec3 fragPos, vec3 texOffset)
{
	for(uint i = startIdx; i < startIdx + countIdx; ++i)
	{
		mat4 trans = getShadowTransform(i);
		vec4 shadowCoord = mul(trans, vec4(fragPos, 1.0));
		vec3 texCoord = shadowCoord.xyz / shadowCoord.w;
		texCoord += texOffset;
		if(!outsideShadowMap(texCoord))
		{
			return getShadowMapValue(i, texCoord);
		}
	}
	return 1.0;
}

vec3 getShadowMapTextureBaseOffset()
{
	float texelSize = u_shadowTexelSize;
	return vec3(texelSize, texelSize, 1.0);
}

float hardShadow(uint startIdx, uint countIdx, vec3 fragPos, float bias)
{
	vec3 offset = getShadowMapTextureBaseOffset();
	return doHardShadow(startIdx, countIdx, fragPos, vec3(0.0, 0.0, -bias) * offset);
}

// percentage closer filtering shadow
// https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf
float softShadow(uint startIdx, uint countIdx, vec3 fragPos, float bias)
{
	float v = 0.0;
	vec3 offset = getShadowMapTextureBaseOffset();

	v += doHardShadow(startIdx, countIdx, fragPos, vec3(-1.5, -1.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(-1.5, -0.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(-1.5,  0.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(-1.5,  1.5, -bias) * offset);

	v += doHardShadow(startIdx, countIdx, fragPos, vec3(-0.5, -1.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(-0.5, -0.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(-0.5,  0.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(-0.5,  1.5, -bias) * offset);

	v += doHardShadow(startIdx, countIdx, fragPos, vec3(0.5, -1.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(0.5, -0.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(0.5,  0.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(0.5,  1.5, -bias) * offset);

	v += doHardShadow(startIdx, countIdx, fragPos, vec3(1.5, -1.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(1.5, -0.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(1.5,  0.5, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, vec3(1.5,  1.5, -bias) * offset);

	return v / 16.0;
}

bool hasSoftShadow(uint i)
{
	return u_shadowSoftMask & (1 << i) != 0;
}

float lightShadow(uint lightIdx, uint startMapIdx, uint countMapIdx, vec3 fragPos, float bias)
{
	if(hasSoftShadow(lightIdx))
	{
		return softShadow(startMapIdx, countMapIdx, fragPos, bias);
	}
	return hardShadow(startMapIdx, countMapIdx, fragPos, bias);
}

// using multiplicative shadow blending like Unity
float shadowVisibility(vec3 fragPos, vec3 normal)
{
	float visibility = 1.0;
	uint offsetMapIdx = 0;
	uint countMapIdx = u_shadowCascadeAmount;
	float bias = 0.0;
	for(uint i = 0; i < u_shadowDirAmout; ++i)
	{
		vec3 dir = b_shadowDirection[i];
		uint startMapIdx = i * countMapIdx;
		bias = normalShadowBias(normal, -dir);
		visibility *= lightShadow(i, startMapIdx, countMapIdx, fragPos, bias);
	}
	offsetMapIdx = u_shadowDirAmout * countMapIdx;
	countMapIdx = 1;
	bias = 0.0;
	for(uint i = 0; i < u_shadowSpotAmount; ++i)
	{
		uint startMapIdx = offsetMapIdx + i;
		uint lightIdx = u_shadowDirAmout + i;
		visibility *= lightShadow(lightIdx, startMapIdx, countMapIdx, fragPos, bias);
	}
	countMapIdx = 6;
	offsetMapIdx += u_shadowSpotAmount;
	for(uint i = 0; i < u_shadowPointAmount; ++i)
	{
		uint startMapIdx = offsetMapIdx + (i * countMapIdx);
		uint lightIdx = u_shadowDirAmout + u_shadowSpotAmount + i;
		visibility *= lightShadow(lightIdx, startMapIdx, countMapIdx, fragPos, bias);
	}
	return visibility;
}

#endif // DARMOK_SHADOW_HEADER