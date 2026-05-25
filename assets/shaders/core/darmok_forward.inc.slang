#pragma once

#include <darmok_material.inc.slang>
#include <darmok_light.inc.slang>
#include <darmok_shadow.inc.slang>
#include <darmok.inc.slang>

float3 calcLightRadiance(LightData data, float3 V, float3 N, float NoV, float3 msFactor, Material mat)
{
    float NoL = saturate(dot(N, data.direction));
    return BRDF(V, data.direction, N, NoV, NoL, mat) * msFactor * data.radiance * NoL;
}

float4 forwardFragment(float3 V, float3 normal, float3 tangent, float3 fragPos, float2 texCoord)
{
    V = normalize(V);
    Material mat = getMaterial(texCoord);

    // convert normal map from tangent space -> world space (= space of tangent, etc.)
    float3 N = convertTangentNormal(normal, tangent, mat.normal);
    mat.a = specularAntiAliasing(N, mat.a);

    float NoV = abs(dot(N, V)) + 1e-5;

    float3 msFactor = multipleScatteringFactor(mat, NoV);
    float3 radianceOut = float3(0.0, 0.0, 0.0);

    if(whiteFurnaceEnabled())
    {
        mat.F0 = float3(1.0, 1.0, 1.0);
        radianceOut += whiteFurnace(NoV, mat) * msFactor;
        return float4(radianceOut, 1.0);
    }

    uint pointLights = pointLightCount();
    for(uint i = 0; i < pointLights; ++i)
    {
        PointLight light = getPointLight(i);
        LightData data = calcPointLight(light, fragPos);
        float visibility = pointLightShadow(light.entity, fragPos, N, data.direction);
        if(visibility <= 0.0) continue;
        radianceOut += calcLightRadiance(data, V, N, NoV, msFactor, mat) * visibility;
    }

    uint spotLights = spotLightCount();
    for(uint i = 0; i < spotLights; ++i)
    {
        SpotLight light = getSpotLight(i);
        LightData data = calcSpotLight(light, fragPos);
        float visibility = spotLightShadow(light.entity, fragPos, N, data.direction);
        if(visibility <= 0.0) continue;
        radianceOut += calcLightRadiance(data, V, N, NoV, msFactor, mat) * visibility;
    }

    uint dirLights = dirLightCount();
    for(uint i = 0; i < dirLights; ++i)
    {
        DirLight light = getDirLight(i);
        LightData data = calcDirLight(light);
        float visibility = dirLightShadow(light.entity, fragPos, N, data.direction);
        if(visibility <= 0.0) continue;
        radianceOut += calcLightRadiance(data, V, N, NoV, msFactor, mat) * visibility;
    }

    radianceOut += getAmbientLight().irradiance * mat.diffuse * mat.occlusion;
    radianceOut += mat.emissive;

    return float4(radianceOut, mat.albedo.a);
}
