#pragma once

#include <darmok_material_basic.inc.slang>
#include <darmok_light.inc.slang>
#include <darmok_shadow.inc.slang>

float3 calcLightRadiance(LightData data, float3 V, float3 N, Material mat)
{
    float3 diffuse = phongDiffuse(data.direction, N, data.radiance);
    float3 specular = phongSpecular(V, data.direction, N, data.radiance, mat.shininess);
    return (diffuse * mat.diffuse.xyz) + (specular * mat.specular);
}

static inline float4 forwardFragment(float3 V, float3 N, float3 fragPos, float2 texCoord)
{
    V = normalize(V);
    N = normalize(N);
    Material mat = getMaterial(texCoord);
    float3 radianceOut = float3(0.0f, 0.0f, 0.0f);

    uint pointLights = pointLightCount();
    for(uint i = 0u; i < pointLights; ++i)
    {
        PointLight light = getPointLight(i);
        LightData data = calcPointLight(light, fragPos);
        float visibility = pointLightShadow(light.entity, fragPos, N, data.direction);
        if(visibility <= 0.0f) continue;
        radianceOut += calcLightRadiance(data, V, N, mat) * visibility;
    }

    uint spotLights = spotLightCount();
    for(uint i = 0u; i < spotLights; ++i)
    {
        SpotLight light = getSpotLight(i);
        LightData data = calcSpotLight(light, fragPos);
        float visibility = spotLightShadow(light.entity, fragPos, N, data.direction);
        if(visibility <= 0.0f) continue;
        radianceOut += calcLightRadiance(data, V, N, mat) * visibility;
    }

    uint dirLights = dirLightCount();
    for(uint i = 0u; i < dirLights; ++i)
    {
        DirLight light = getDirLight(i);
        LightData data = calcDirLight(light);
        float visibility = dirLightShadow(light.entity, fragPos, N, data.direction);
        if(visibility <= 0.0f) continue;
        radianceOut += calcLightRadiance(data, V, N, mat) * visibility;
    }

    radianceOut += getAmbientLight().irradiance * mat.diffuse.xyz;
    return float4(radianceOut, mat.diffuse.w);
}


