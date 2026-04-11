#pragma once

// for each point light:
//   vec4 position + uint entity
//   vec4 intensity + range (xyz is intensity, w is range)
Texture2D<float4> b_pointLights;

// for each spot light:
//   vec3 position + uint entity
//   vec4 direction (w is padding)
//   vec4 intensity + range (xyz is intensity, w is range)
//   vec4 cone angle, inner cone angle
Texture2D<float4> b_spotLights;

// for each directional light:
//   vec3 direction + uint entity
//   vec4 intensity (xyz is intensity)
Texture2D<float4> b_dirLights;

cbuffer LightConstants
{
    float4 u_lightCountVec;
    float4 u_ambientLightIrradiance;
}

#define u_pointLightCount uint(u_lightCountVec.x)
#define u_dirLightCount   uint(u_lightCountVec.y)
#define u_spotLightCount  uint(u_lightCountVec.z)

struct PointLight
{
    uint entity;
    float3 position;
    float3 intensity;
    float range;
};

struct SpotLight
{
    uint entity;
    float3 position;
    float3 direction;
    float3 intensity;
    float range;
    float coneAngle;
    float innerConeAngle;
};

struct DirLight
{
    uint entity;
    float3 direction;
    float3 intensity;
};

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
    PointLight light;
    i *= 2u;
    float4 posVec = b_pointLights.Load(int3(i+0,0,0));
    light.entity = (uint)posVec.w;
    light.position = posVec.xyz;
    float4 intensityRangeVec = b_pointLights.Load(int3(i+1,0,0));
    light.intensity = intensityRangeVec.xyz;
    light.range = intensityRangeVec.w;
    return light;
}

uint spotLightCount()
{
    return u_spotLightCount;
}

SpotLight getSpotLight(uint i)
{
    SpotLight light;
    i *= 4u;
    float4 posVec = b_spotLights.Load(int3(i+0,0,0));
    light.entity = (uint)posVec.w;
    light.position = posVec.xyz;
    light.direction = b_spotLights.Load(int3(i+1,0,0)).xyz;
    float4 intensityRangeVec = b_spotLights.Load(int3(i+2,0,0));
    light.intensity = intensityRangeVec.xyz;
    light.range = intensityRangeVec.w;
    float4 dataVec = b_spotLights.Load(int3(i+3,0,0));
    light.coneAngle = dataVec.x;
    light.innerConeAngle = dataVec.y;
    return light;
}

uint dirLightCount()
{
    return u_dirLightCount;
}

DirLight getDirLight(uint i)
{
    DirLight light;
    i *= 2u;
    float4 posVec = b_dirLights.Load(int3(i+0,0,0));
    light.entity = (uint)posVec.w;
    light.direction = posVec.xyz;
    float4 intensityVec = b_dirLights.Load(int3(i+1,0,0));
    light.intensity = intensityVec.xyz;
    return light;
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
    data.direction = data.direction / dist;
    float attenuation = smoothAttenuation(dist, light.range);
    data.radiance = light.intensity * attenuation;
    return data;
}

LightData calcSpotLight(SpotLight light, float3 pos)
{
    LightData data; data.radiance = float3(0.0,0.0,0.0);
    data.direction = light.position - pos;
    float dist = length(data.direction);
    data.direction = data.direction / dist;
    float cosTheta = dot(data.direction, -light.direction);
    float innerCutoff = cos(light.innerConeAngle);
    float outerCutoff = cos(light.coneAngle);
    if (cosTheta > outerCutoff)
    {
        float factor = (cosTheta - outerCutoff) / (innerCutoff - outerCutoff);
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
