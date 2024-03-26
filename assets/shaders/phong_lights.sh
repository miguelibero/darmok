#ifndef PHONG_LIGHTS_SH_HEADER_GUARD
#define PHONG_LIGHTS_SH_HEADER_GUARD

#include <bgfx_compute.sh>
#include "samplers.sh"

uniform vec4 u_lightCount;
#define u_pointLightCount uint(u_lightCount.x)

uniform vec4 u_ambientLightColor;

// for each point light:
//   vec3 position
//   vec3 diffuse color
//   vec3 specular color
// SAMPLER_LIGHTS_POINTLIGHTS=6
BUFFER_RO(b_pointLights, vec4, 6);

struct PointLight
{
    vec3 position;
    vec3 diffuse;
    vec3 specular;
};

struct AmbientLight
{
    vec3 color;
};

uint pointLightCount()
{
    return u_pointLightCount;
}

PointLight getPointLight(uint i)
{
    PointLight light;
    light.position = b_pointLights[3 * i + 0].xyz;
    light.diffuse = b_pointLights[3 * i + 1].xyz;
    light.specular = b_pointLights[3 * i + 2].xyz;
    return light;
}

AmbientLight getAmbientLight()
{
    AmbientLight light;
    light.color = u_ambientLightColor.xyz;
    return light;
}

#endif // PHONG_LIGHTS_SH_HEADER_GUARD