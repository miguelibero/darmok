#ifndef DARMOK_LIGHTS_HEADER
#define DARMOK_LIGHTS_HEADER

#include <bgfx_compute.sh>
#include <darmok_sampler.sc>

uniform vec4 u_lightCountVec;
#define u_pointLightCount uint(u_lightCountVec.x)
#define u_dirLightCount   uint(u_lightCountVec.y)

uniform vec4 u_ambientLightIrradiance;

// for each point light:
//   vec4 position (w is padding)
//   vec4 intensity + radius (xyz is intensity, w is radius)
BUFFER_RO(b_pointLights, vec4, DARMOK_SAMPLER_LIGHTS_POINT);

// for each directional light:
//   vec4 direction (w is padding)
//   vec4 intensity (xyz is intensity)
BUFFER_RO(b_dirLights, vec4, DARMOK_SAMPLER_LIGHTS_DIR);

struct PointLight
{
    vec3 position;
    vec3 intensity;
    float radius;
};

struct DirectionalLight
{
    vec3 direction;
    vec3 intensity;
    mat4 trans;
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

float smoothAttenuation(float distance, float radius)
{
    // window function with smooth transition to 0
    // radius is arbitrary (and usually artist controlled)
    float nom = saturate(1.0 - pow(distance / radius, 4.0));
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
    vec4 intensityRadiusVec = b_pointLights[i + 1];
    light.intensity = intensityRadiusVec.xyz;
    light.radius = intensityRadiusVec.w;
    return light;
}

uint dirLightCount()
{
    return u_dirLightCount;
}

DirectionalLight getDirLight(uint i)
{
    DirectionalLight light;
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

#endif // DARMOK_LIGHTS_HEADER
