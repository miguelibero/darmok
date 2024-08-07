#ifndef DARMOK_PHONG_LIGHTING_HEADER
#define DARMOK_PHONG_LIGHTING_HEADER

#include <bgfx_compute.sh>

uniform vec4 u_lightCount;
uniform vec4 u_lightingData;
uniform vec4 u_material;

#define u_pointLightCount uint(u_lightCount.x)

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

uint getPointLightCount()
{
    return u_pointLightCount;
}

PointLight getPointLight(uint i)
{
    PointLight light;
    i *= 3;
    light.position  = b_pointLights[i + 0].xyz;
    light.diffuse   = b_pointLights[i + 1].xyz;
    light.specular  = b_pointLights[i + 2].xyz;
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
    mat.shininess = int(u_material[0]);
    mat.specularStrength = u_material[1];
    return mat;
}

#endif // DARMOK_PHONG_LIGHTING_HEADER