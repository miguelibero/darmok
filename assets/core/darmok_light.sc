#ifndef DARMOK_LIGHTS_HEADER
#define DARMOK_LIGHTS_HEADER

#include <bgfx_compute.sh>
#include <darmok_sampler.sc>

uniform vec4 u_lightCountVec;
#define u_pointLightCount uint(u_lightCountVec.x)
#define u_dirLightCount   uint(u_lightCountVec.y)
#define u_spotLightCount   uint(u_lightCountVec.z)

uniform vec4 u_ambientLightIrradiance;

// for each point light:
//   vec4 position (w is padding)
//   vec4 intensity + range (xyz is intensity, w is range)
BUFFER_RO(b_pointLights, vec4, DARMOK_SAMPLER_LIGHTS_POINT);

// for each spot light:
//   vec4 position (w is padding)
//   vec4 direction (w is padding)
//   vec4 intensity + range (xyz is intensity, w is range)
//   vec4 cone angle, inner cone angle
BUFFER_RO(b_spotLights, vec4, DARMOK_SAMPLER_LIGHTS_SPOT);

// for each directional light:
//   vec4 direction (w is padding)
//   vec4 intensity (xyz is intensity)
BUFFER_RO(b_dirLights, vec4, DARMOK_SAMPLER_LIGHTS_DIR);

struct PointLight
{
    vec3 position;
    vec3 intensity;
    float range;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    vec3 intensity;
    float range;
    float coneAngle;
    float innerConeAngle;
};

struct DirLight
{
    vec3 direction;
    vec3 intensity;
};

struct AmbientLight
{
    vec3 irradiance;
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
    i *= 2;
    light.position = b_pointLights[i + 0].xyz;
    vec4 intensityRangeVec = b_pointLights[i + 1];
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
    i *= 4;
    light.position = b_spotLights[i + 0].xyz;
    light.direction = b_spotLights[i + 1].xyz;
    vec4 intensityRangeVec = b_spotLights[i + 2];
    light.intensity = intensityRangeVec.xyz;
    light.range = intensityRangeVec.w;
    vec4 dataVec = b_spotLights[i + 3];
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
    i *= 2;
    light.direction = b_dirLights[i + 0].xyz;
    vec4 intensityVec = b_dirLights[i + 1];
    light.intensity = intensityVec.xyz;
    return light;
}

AmbientLight getAmbientLight()
{
    AmbientLight light;
    light.irradiance = u_ambientLightIrradiance.xyz;
    return light;
}

struct LightData
{
    vec3 radiance;
    vec3 direction;
};

LightData calcPointLight(PointLight light, vec3 pos)
{
    LightData data;
    data.direction = light.position - pos;
    float dist = length(data.direction);
    data.direction = data.direction / dist;
    float attenuation = smoothAttenuation(dist, light.range);
    data.radiance = light.intensity * attenuation;
    return data;
}

LightData calcSpotLight(SpotLight light, vec3 pos)
{
    LightData data;
    data.radiance = 0.0;
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
        if(attenuation > 0.0)
        {
            data.radiance = light.intensity * attenuation;
        }
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

#endif // DARMOK_LIGHTS_HEADER
