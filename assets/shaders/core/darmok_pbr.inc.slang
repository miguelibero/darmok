#pragma once

#include <darmok_material.inc.slang>
#include <darmok_light.inc.slang>
#include <darmok_shadow.inc.slang>
#include <darmok.inc.slang>

[[vk::binding(6, 0)]]
TextureCube<float4> s_irradiance : register(t6);
[[vk::binding(6, 0)]]
uniform SamplerState s_irradianceState : register(s6);

[[vk::binding(7, 0)]]
TextureCube<float4> s_prefilteredEnv : register(t7);
[[vk::binding(7, 0)]]
uniform SamplerState s_prefilteredEnvState : register(s7);

[[vk::binding(8, 0)]]
uniform Texture2D s_albedoLut : register(t8);
[[vk::binding(8, 0)]]
SamplerState s_albedoLutState : register(s8);

[[vk::binding(9, 0)]]
uniform Texture2D<float2> s_brdfLut : register(t9);
[[vk::binding(9, 0)]]
uniform SamplerState s_brdfLutState : register(s9);

uniform float4 u_multipleScatteringVec;

#define u_multipleScattering   (u_multipleScatteringVec.x != 0.0)
#define u_whiteFurnaceRadiance (u_multipleScatteringVec.y)
#define u_whiteFurnace         (u_whiteFurnaceRadiance > 0.0)

bool whiteFurnaceEnabled()
{
    return u_whiteFurnace;
}

// White furnace test: lighting integral against a constant white environment
// Used to visualize energy loss/gain
// This is exactly what the multiple scattering LUT calculates
float3 whiteFurnace(float NoV, Material mat)
{
    float2 Es = s_albedoLut.Sample(s_albedoLutState, float2(NoV, mat.a)).xy;
    float E = lerp(Es.y, Es.x, mat.metallic);
    return E * float3(u_whiteFurnaceRadiance, u_whiteFurnaceRadiance, u_whiteFurnaceRadiance);
}


// Turquin. 2018. Practical multiple scattering compensation for microfacet models.
// https://blog.selfshadow.com/publications/turquin/ms_comp_final.pdf
float3 multipleScatteringFactor(Material mat, float NoV)
{
    if(u_multipleScattering)
    {
        // Turquin approximates the multiple scattering portion of the BRDF using a scaled down version of the single scattering BRDF
        // That scale factor is E: the directional albedo for single scattering, ie. the total reflectance for a viewing direction
        float2 E = s_albedoLut.Sample(s_albedoLutState, float2(NoV, mat.a)).xy;

        // for metals, the albedo value is calculated with F = 1 (perfect reflection)
        // fresnel determines whether light is reflected or absorbed
        float3 factorMetallic = float3(1.0, 1.0, 1.0) + mat.F0 * (1.0 / E.x - 1.0);

        // for dielectrics, fresnel determines the ratio between specular and diffuse energy
        // so the albedo depends on F as a variable
        // however, dielectrics in GLTF have a fixed F0 of 0.04 so we can do this with a second LUT
        float3 factorDielectric = float3(1.0 / E.y, 1.0 / E.y, 1.0 / E.y);

        return lerp(factorDielectric, factorMetallic, mat.metallic);
    }
    else return float3(1.0, 1.0, 1.0);
}

float3 calcLightRadiance(LightData data, float3 V, float3 N, float NoV, float3 msFactor, Material mat)
{
    float NoL = saturate(dot(N, data.direction));
    return BRDF(V, data.direction, N, NoV, NoL, mat) * msFactor * data.radiance * NoL;
}

float3 calcAmbientIBL(
    Material mat,
    float3 N,
    float3 V,
    float NoV)
{
    float3 irradiance =
        s_irradiance.Sample(s_irradianceState, N).rgb;

    float3 diffuse =
        irradiance * mat.diffuse;

    float3 R = reflect(-V, N);

    float mip = mat.roughness * u_environmentMaxMip;

    float3 environment =
        s_prefilteredEnv.SampleLevel(
            s_prefilteredEnvState,
            R,
            mip
        ).rgb;

    float2 brdf =
        s_brdfLut.Sample(
            s_brdfLutState,
            float2(NoV, mat.roughness)
        );

    float3 specular =
        environment * (mat.F0 * brdf.x + brdf.y);

    return diffuse + specular;
}

float4 applyFragmentLights(float3 V, float3 N, float3 fragPos, Material mat)
{
    V = normalize(V);
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
        float visibility = pointLightShadow(light.shadow, fragPos, N, data.direction);
        if(visibility <= 0.0)
        {
            continue;
        }
        radianceOut += calcLightRadiance(data, V, N, NoV, msFactor, mat) * visibility;
    }

    uint spotLights = spotLightCount();
    for(uint i = 0; i < spotLights; ++i)
    {
        SpotLight light = getSpotLight(i);
        LightData data = calcSpotLight(light, fragPos);
        float visibility = spotLightShadow(light.shadow, fragPos, N, data.direction);
        if(visibility <= 0.0)
        {
            continue;
        }
        radianceOut += calcLightRadiance(data, V, N, NoV, msFactor, mat) * visibility;
    }

    uint dirLights = dirLightCount();
    for(uint i = 0; i < dirLights; ++i)
    {
        DirLight light = getDirLight(i);
        LightData data = calcDirLight(light);
        float visibility = dirLightShadow(light.shadow, fragPos, N, data.direction);
        if(visibility <= 0.0)
        {
            continue;
        }
        radianceOut += calcLightRadiance(data, V, N, NoV, msFactor, mat) * visibility;
    }

    radianceOut += calcAmbientIBL(mat, N, V, NoV) * getAmbientLight().irradiance * mat.occlusion;
    radianceOut += mat.emissive;

    return float4(radianceOut, mat.albedo.a);
}
