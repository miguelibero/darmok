#pragma once

enum LightType
{
    Dir = 0,
    Spot = 1,
    Point = 2,
    Ambient = 3,
};

enum ShadowType
{
    None = 0,
    Hard = 1,
    Soft = 2,
};

struct LightShadowData
{
	uint shadowType;
	uint shadowMapIndex;
};

// Use a struct of scalars instead of float3 for StructuredBuffer elements.
// Metal cannot resolve the address space of float3 in device buffers
// (ambiguous between packed_float3 and float3), causing a Slang internal error.
struct PackedVec3
{
    float x, y, z;
};

float3 toFloat3(PackedVec3 v) { return float3(v.x, v.y, v.z); }

struct DirLight
{
    PackedVec3 direction;
    PackedVec3 intensity;

    LightShadowData shadow;
};

[[vk::binding(10, 0)]]
uniform StructuredBuffer<DirLight> b_dirLights : register(t10);

struct PointLight
{
    PackedVec3 position;
    PackedVec3 intensity;
    float range;

    LightShadowData shadow;
};

[[vk::binding(11, 0)]]
uniform StructuredBuffer<PointLight> b_pointLights : register(t11);

struct SpotLight
{
    PackedVec3 position;
    PackedVec3 direction;
    PackedVec3 intensity;
    float range;
    float coneAngle;
    float innerConeAngle;

    LightShadowData shadow;
};

[[vk::binding(12, 0)]]
uniform StructuredBuffer<SpotLight> b_spotLights : register(t12);

uniform float4 u_lightCountVec;
uniform float4 u_ambientLightIrradiance;

#define u_dirLightCount   uint(u_lightCountVec.x)
#define u_pointLightCount uint(u_lightCountVec.y)
#define u_spotLightCount  uint(u_lightCountVec.z)

struct AmbientLight
{
    float3 irradiance;
};

float distanceAttenuation(float distance)
{
    return 1.0 / max(distance * distance, 0.01 * 0.01);
}

float smoothAttenuation(float distance, float range)
{
    float nom = saturate(1.0 - pow(distance / range, 4.0));
    return nom * nom * distanceAttenuation(distance);
}

uint pointLightCount()
{
    return u_pointLightCount;
}

PointLight getPointLight(uint i)
{
    return b_pointLights[i];
}

uint spotLightCount()
{
    return u_spotLightCount;
}

SpotLight getSpotLight(uint i)
{
    return b_spotLights[i];
}

uint dirLightCount()
{
    return u_dirLightCount;
}

DirLight getDirLight(uint i)
{
    return b_dirLights[i];
}

AmbientLight getAmbientLight()
{
    AmbientLight l;
    l.irradiance = u_ambientLightIrradiance.xyz;
    return l;
}

struct LightData
{
    float3 radiance;
    float3 direction;
};

LightData calcPointLight(PointLight light, float3 pos)
{
    LightData data;
    data.direction = toFloat3(light.position) - pos;
    float dist = length(data.direction);
    data.direction = data.direction / max(dist, 1e-5);
    float attenuation = smoothAttenuation(dist, light.range);
    data.radiance = toFloat3(light.intensity) * attenuation;
    return data;
}

LightData calcSpotLight(SpotLight light, float3 pos)
{
    LightData data; data.radiance = float3(0.0, 0.0, 0.0);
    data.direction = toFloat3(light.position) - pos;
    float dist = length(data.direction);
    data.direction = data.direction / max(dist, 1e-5);
    float cosTheta = dot(data.direction, -toFloat3(light.direction));
    float innerCutoff = cos(light.innerConeAngle);
    float outerCutoff = cos(light.coneAngle);
    if (cosTheta > outerCutoff)
    {
        float factor = saturate(
            (cosTheta - outerCutoff) /
            max(innerCutoff - outerCutoff, 1e-5)
        );
        float attenuation = factor * smoothAttenuation(dist, light.range);
        if(attenuation > 0.0) data.radiance = toFloat3(light.intensity) * attenuation;
    }
    return data;
}

LightData calcDirLight(DirLight light)
{
    LightData data;
    data.direction = normalize(-toFloat3(light.direction));
    data.radiance = toFloat3(light.intensity);
    return data;
}
