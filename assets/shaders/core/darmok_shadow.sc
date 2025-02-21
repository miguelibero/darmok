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
#define u_shadowMapAmount		uint(u_shadowData2.w)

#if DARMOK_VARIANT_SHADOW_ENABLED
// light shadow maps ordered by type: directional (cascaded), spot, point
SAMPLER2DARRAYSHADOW(s_shadowMap, DARMOK_SAMPLER_SHADOW_MAP);
#define Sampler sampler2DShadowArray
#endif

// for each shadow map:
//   mat4 transform
BUFFER_RO(b_shadowTrans, vec4, DARMOK_SAMPLER_SHADOW_TRANS);

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
BUFFER_RO(b_shadowLightData, vec4, DARMOK_SAMPLER_SHADOW_LIGHT_DATA);

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

#if DARMOK_VARIANT_SHADOW_ENABLED
float getShadowMapValue(uint shadowMapIndex, vec3 texCoord)
{
	return shadow2DArray(s_shadowMap, vec4(texCoord.xy, shadowMapIndex, texCoord.z)).x;
}
#else
float getShadowMapValue(uint shadowMapIndex, vec3 texCoord)
{
	return 1.0;
}
#endif

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

struct EntityShadowData
{
	uint shadowType;
	uint startMapIndex;
};

EntityShadowData getEntityShadowData(uint entity, uint lightType)
{
	EntityShadowData data;
	data.shadowType = SHADOW_TYPE_NONE;
	for(uint i = 0; i < u_shadowMapAmount; ++i)
	{
		vec4 elm = b_shadowLightData[i];
		if(uint(elm.x) == entity && uint(elm.y) == lightType)
		{
			data.shadowType = uint(elm.z);
			data.startMapIndex = i;
			return data;
		}
	}
	return data;
}

float lightShadow(EntityShadowData data, uint countMapIdx, vec3 fragPos, float bias)
{
	switch(data.shadowType)
	{
		case SHADOW_TYPE_HARD:
			return hardShadow(data.startMapIndex, countMapIdx, fragPos, bias);
		case SHADOW_TYPE_SOFT:
			return softShadow(data.startMapIndex, countMapIdx, fragPos, bias);
	}
	return 1.0;
}

float dirLightShadow(uint entity, vec3 fragPos, vec3 normal, vec3 dir)
{
	EntityShadowData data = getEntityShadowData(entity, SHADOW_LIGHT_TYPE_DIR);
	if(data.shadowType == SHADOW_TYPE_NONE)
	{
		return 1.0;
	}
	float bias = normalShadowBias(normal, dir);
	return lightShadow(data, u_shadowCascadeAmount, fragPos, bias);
}

float spotLightShadow(uint entity, vec3 fragPos, vec3 normal, vec3 dir)
{
	EntityShadowData data = getEntityShadowData(entity, SHADOW_LIGHT_TYPE_SPOT);
	if(data.shadowType == SHADOW_TYPE_NONE)
	{
		return 1.0;
	}
	float bias = normalShadowBias(normal, dir);
	return lightShadow(data, 1, fragPos, bias);
}

float pointLightShadow(uint entity, vec3 fragPos, vec3 normal, vec3 dir)
{
	EntityShadowData data = getEntityShadowData(entity, SHADOW_LIGHT_TYPE_POINT);
	if(data.shadowType == SHADOW_TYPE_NONE)
	{
		return 1.0;
	}
	float bias = normalShadowBias(normal, dir);
	return lightShadow(data, 6, fragPos, bias);
}

#endif // DARMOK_SHADOW_HEADER