
#pragma once

#include "darmok_light.inc.slang"

float4 u_shadowData1; // x: texel size, y: cascade amount, z: bias, w: normal bias
float4 u_shadowData2; // x: dir amount, y: spot amount, z: point amount, w: map amount

#define u_shadowTexelSize        u_shadowData1.x
#define u_shadowCascadeAmount    uint(u_shadowData1.y)
#define u_shadowBias             u_shadowData1.z
#define u_shadowNormalBias       u_shadowData1.w

#define u_shadowDirAmount        uint(u_shadowData2.x)
#define u_shadowSpotAmount       uint(u_shadowData2.y)
#define u_shadowPointAmount      uint(u_shadowData2.z)
#define u_shadowMapAmount        uint(u_shadowData2.w)

#ifdef DARMOK_VARIANT_SHADOW_ENABLED

// light shadow maps ordered by type: directional (cascaded), spot, point
Texture2DArray<float> s_shadowMap;
SamplerComparisonState s_shadowMapCmp;

#endif

// for each shadow map:
//   mat4 transform
StructuredBuffer<float4x4> b_shadowTrans;

#define SHADOW_LIGHT_TYPE_DIR 0
#define SHADOW_LIGHT_TYPE_SPOT 1
#define SHADOW_LIGHT_TYPE_POINT 2

#define SHADOW_TYPE_NONE 0
#define SHADOW_TYPE_HARD 1
#define SHADOW_TYPE_SOFT 2

// for each shadow map:
//   uint entity
//   uint light type (0 = Dir, 1 = Spot, 2 = Point)
//   uint shadow type (0 = None, 1 = Hard, 2 = Soft)
StructuredBuffer<float4> b_shadowLightData;

float4x4 getShadowTransform(uint shadowMapIndex)
{
	return b_shadowTrans[shadowMapIndex];
}

bool outsideShadowMap(float3 texCoord)
{
	return (texCoord.x > 1.0f) || (texCoord.y > 1.0f) || (texCoord.z > 1.0f)
		|| (texCoord.x < 0.0f) || (texCoord.y < 0.0f) || (texCoord.z < 0.0f);
}

#ifdef DARMOK_VARIANT_SHADOW_ENABLED

float getShadowMapValue(uint shadowMapIndex, float3 texCoord)
{
	// SampleCmp expects float3 coords: (u,v,layer) and a comparison value
	float cmp = texCoord.z;
	float3 coord = float3(texCoord.xy, (float)shadowMapIndex);
	return s_shadowMap.SampleCmp(s_shadowMapCmp, coord, cmp);
}

#else

float getShadowMapValue(uint /*shadowMapIndex*/, float3 /*texCoord*/)
{
	return 1.0f;
}

#endif

float normalShadowBias(float3 norm, float3 lightDir)
{
	return max(u_shadowNormalBias * (1.0f - dot(norm, lightDir)), u_shadowBias);
}

float doHardShadow(uint startIdx, uint countIdx, float3 fragPos, float3 texOffset)
{
	for(uint i = startIdx; i < startIdx + countIdx; ++i)
	{
		float4x4 trans = getShadowTransform(i);
		float4 shadowCoord = mul(trans, float4(fragPos, 1.0f));
		float3 texCoord = shadowCoord.xyz / shadowCoord.w;
		texCoord += texOffset;
		if(!outsideShadowMap(texCoord))
		{
			return getShadowMapValue(i, texCoord);
		}
	}
	return 1.0f;
}

float3 getShadowMapTextureBaseOffset()
{
	float texelSize = u_shadowTexelSize;
	return float3(texelSize, texelSize, 1.0f);
}

float hardShadow(uint startIdx, uint countIdx, float3 fragPos, float bias)
{
	float3 offset = getShadowMapTextureBaseOffset();
	return doHardShadow(startIdx, countIdx, fragPos, float3(0.0f, 0.0f, -bias) * offset);
}

// percentage closer filtering shadow
// https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf
float softShadow(uint startIdx, uint countIdx, float3 fragPos, float bias)
{
	float v = 0.0f;
	float3 offset = getShadowMapTextureBaseOffset();

	v += doHardShadow(startIdx, countIdx, fragPos, float3(-1.5f, -1.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(-1.5f, -0.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(-1.5f,  0.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(-1.5f,  1.5f, -bias) * offset);

	v += doHardShadow(startIdx, countIdx, fragPos, float3(-0.5f, -1.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(-0.5f, -0.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(-0.5f,  0.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(-0.5f,  1.5f, -bias) * offset);

	v += doHardShadow(startIdx, countIdx, fragPos, float3(0.5f, -1.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(0.5f, -0.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(0.5f,  0.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(0.5f,  1.5f, -bias) * offset);

	v += doHardShadow(startIdx, countIdx, fragPos, float3(1.5f, -1.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(1.5f, -0.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(1.5f,  0.5f, -bias) * offset);
	v += doHardShadow(startIdx, countIdx, fragPos, float3(1.5f,  1.5f, -bias) * offset);

	return v / 16.0f;
}

struct EntityShadowData
{
	uint shadowType;
	uint startMapIndex;
};

EntityShadowData getEntityShadowData(uint entity, uint lightType)
{
	EntityShadowData data;
	data.shadowType = SHADOW_TYPE_NONE;
	data.startMapIndex = 0u;
	for(uint i = 0u; i < u_shadowMapAmount; ++i)
	{
		float4 elm = b_shadowLightData[i];
		if((uint)elm.x == entity && (uint)elm.y == lightType)
		{
			data.shadowType = (uint)elm.z;
			data.startMapIndex = i;
			return data;
		}
	}
	return data;
}

float lightShadow(EntityShadowData data, uint countMapIdx, float3 fragPos, float bias)
{
	if(data.shadowType == SHADOW_TYPE_HARD)
		return hardShadow(data.startMapIndex, countMapIdx, fragPos, bias);
	if(data.shadowType == SHADOW_TYPE_SOFT)
		return softShadow(data.startMapIndex, countMapIdx, fragPos, bias);
	return 1.0f;
}

float dirLightShadow(uint entity, float3 fragPos, float3 normal, float3 dir)
{
	EntityShadowData data = getEntityShadowData(entity, SHADOW_LIGHT_TYPE_DIR);
	if(data.shadowType == SHADOW_TYPE_NONE) return 1.0f;
	float bias = normalShadowBias(normal, dir);
	return lightShadow(data, u_shadowCascadeAmount, fragPos, bias);
}

float spotLightShadow(uint entity, float3 fragPos, float3 normal, float3 dir)
{
	EntityShadowData data = getEntityShadowData(entity, SHADOW_LIGHT_TYPE_SPOT);
	if(data.shadowType == SHADOW_TYPE_NONE) return 1.0f;
	float bias = normalShadowBias(normal, dir);
	return lightShadow(data, 1u, fragPos, bias);
}

float pointLightShadow(uint entity, float3 fragPos, float3 normal, float3 dir)
{
	EntityShadowData data = getEntityShadowData(entity, SHADOW_LIGHT_TYPE_POINT);
	if(data.shadowType == SHADOW_TYPE_NONE) return 1.0f;
	float bias = normalShadowBias(normal, dir);
	return lightShadow(data, 6u, fragPos, bias);
}
