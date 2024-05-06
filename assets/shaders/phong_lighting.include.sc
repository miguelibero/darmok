#ifndef PHONG_LIGHTS_SH_HEADER_GUARD
#define PHONG_LIGHTS_SH_HEADER_GUARD

#include <bgfx_compute.sh>
#include "samplers.include.sc"

uniform vec4 u_lightCount;
#define u_pointLightCount uint(u_lightCount.x)

uniform vec4 u_lightingData;

uniform vec4 u_material;

struct PointLight
{
    vec3 position;
    vec3 diffuse;
    vec3 specular;
};

BUFFER_RO(b_pointLights, vec4, 6);

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
    i *= 3;
    light.position  = b_pointLights[i + 0].xyz;
    light.diffuse   = b_pointLights[i + 1].xyz;
    light.specular   = b_pointLights[i + 2].xyz;
    return light;
}

AmbientLight getAmbientLight()
{
    AmbientLight light;
    light.color = u_lightingData.xyz;
    return light;
}

struct Material
{
    int shininess;
    float specularStrength;
};

Material getMaterial()
{
    Material mat;
    mat.shininess = u_material[0];
    mat.specularStrength = u_material[1];
    return mat;
}

#endif // PHONG_LIGHTS_SH_HEADER_GUARD