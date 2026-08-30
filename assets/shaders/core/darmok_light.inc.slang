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
	ShadowType shadowType;
	uint shadowMapIndex;
};

struct DirLight
{
    float3 direction;
    float3 intensity;

    LightShadowData shadow;
};

[[vk::binding(7, 0)]]
uniform StructuredBuffer<DirLight> b_dirLights : register(t7);

struct PointLight
{
    float3 position;
    float3 intensity;
    float range;

    LightShadowData shadow;
};

[[vk::binding(8, 0)]]
uniform StructuredBuffer<PointLight> b_pointLights : register(t8);


struct SpotLight
{
    float3 position;
    float3 direction;
    float3 intensity;
    float range;
    float coneAngle;
    float innerConeAngle;

    LightShadowData shadow;
};

[[vk::binding(9, 0)]]
uniform StructuredBuffer<SpotLight> b_spotLights : register(t9);

uniform float4 u_lightCountVec;
uniform float4 u_ambientLightIrradiance;

#define u_dirLightCount   uint(u_lightCountVec.x)
#define u_pointLightCount uint(u_lightCountVec.y)
#define u_spotLightCount  uint(u_lightCountVec.z)

struct AmbientLight
{
    float3 irradiance;
};

// primary source:
// https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
// also really good:
// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf


float distanceAttenuation(float distance)
{
    // only for point lights
    // physics: inverse square falloff
    // to keep irradiance from reaching infinity at really close distances, stop at 1cm
    return 1.0 / max(distance * distance, 0.01 * 0.01);
}

float smoothAttenuation(float distance, float range)
{
    // window function with smooth transition to 0
    // range is arbitrary (and usually artist controlled)
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
    data.direction = light.position - pos;
    float dist = length(data.direction);
    data.direction = data.direction /  max(dist, 1e-5);
    float attenuation = smoothAttenuation(dist, light.range);
    data.radiance = light.intensity * attenuation;
    return data;
}

LightData calcSpotLight(SpotLight light, float3 pos)
{
    LightData data; data.radiance = float3(0.0,0.0,0.0);
    data.direction = light.position - pos;
    float dist = length(data.direction);
    data.direction = data.direction / max(dist, 1e-5);
    float cosTheta = dot(data.direction, -light.direction);
    float innerCutoff = cos(light.innerConeAngle);
    float outerCutoff = cos(light.coneAngle);
    if (cosTheta > outerCutoff)
    {
        float factor = saturate(
            (cosTheta - outerCutoff) /
            max(innerCutoff - outerCutoff, 1e-5)
        );
        float attenuation = factor * smoothAttenuation(dist, light.range);
        if(attenuation > 0.0) data.radiance = light.intensity * attenuation;
    }
    return data;
}

LightData calcDirLight(DirLight light)
{
    LightData data;
    data.direction = normalize(-light.direction);
    data.radiance = light.intensity;
    return data;
}
