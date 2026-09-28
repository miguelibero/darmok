#pragma once

#include <darmok_brdf.inc.slang>

[[vk::binding(0, 0)]]
uniform Texture2D s_texBaseColor : register(t0);
[[vk::binding(0, 0)]]
SamplerState s_texBaseColorState : register(s0);

[[vk::binding(2, 0)]]
uniform Texture2D s_texMetallicRoughness : register(t2);
[[vk::binding(2, 0)]]
SamplerState s_texMetallicRoughnessState : register(s2);

[[vk::binding(3, 0)]]
uniform Texture2D s_texNormal : register(t3);
[[vk::binding(3, 0)]]
SamplerState s_texNormalState : register(s3);

[[vk::binding(4, 0)]]
uniform Texture2D s_texOcclusion : register(t4);
[[vk::binding(4, 0)]]
SamplerState s_texOcclusionState : register(s4);

[[vk::binding(5, 0)]]
uniform Texture2D s_texEmissive : register(t5);
[[vk::binding(5, 0)]]
SamplerState s_texEmissiveState : register(s5);

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

uniform float4 u_baseColorFactor;
uniform float4 u_metallicRoughnessNormalOcclusionFactor;
uniform float4 u_emissiveFactorVec;
uniform float4 u_hasTextures;
uniform float4 u_multipleScatteringVec;

#define u_hasBaseColorTexture         ((uint(u_hasTextures.x) & (1 << 0)) != 0)
#define u_hasMetallicRoughnessTexture ((uint(u_hasTextures.x) & (1 << 2)) != 0)
#define u_hasNormalTexture            ((uint(u_hasTextures.x) & (1 << 3)) != 0)
#define u_hasOcclusionTexture         ((uint(u_hasTextures.x) & (1 << 4)) != 0)
#define u_hasEmissiveTexture          ((uint(u_hasTextures.x) & (1 << 5)) != 0)

#define u_metallicRoughnessFactor (u_metallicRoughnessNormalOcclusionFactor.xy)
#define u_normalScale             (u_metallicRoughnessNormalOcclusionFactor.z)
#define u_occlusionStrength       (u_metallicRoughnessNormalOcclusionFactor.w)
#define u_emissiveFactor          (u_emissiveFactorVec.xyz)
#define u_environmentMaxMip       (u_emissiveFactorVec.w)

#define u_multipleScattering   (u_multipleScatteringVec.x != 0.0)
#define u_whiteFurnaceRadiance (u_multipleScatteringVec.y)
#define u_whiteFurnace         (u_whiteFurnaceRadiance > 0.0)

float4 materialAlbedoValue(float2 texcoord)
{
    if(u_hasBaseColorTexture)
    {
        return s_texBaseColor.Sample(s_texBaseColorState, texcoord) * u_baseColorFactor;
    }
    else
    {
        return u_baseColorFactor;
    }
}

float2 materialMetallicRoughnessValue(float2 texcoord)
{
    if(u_hasMetallicRoughnessTexture)
    {
        float4 v = s_texMetallicRoughness.Sample(s_texMetallicRoughnessState, texcoord);
        return float2(v.b, v.g) * u_metallicRoughnessFactor;
    }
    else
    {
        return u_metallicRoughnessFactor;
    }
}

float3 materialNormalValue(float2 texcoord)
{
    if(u_hasNormalTexture)
    {
        // the normal scale can cause problems and serves no real purpose
        // normal compression and BRDF calculations assume unit length
        float3 n = s_texNormal.Sample(s_texNormalState, texcoord).rgb * 2.0 - 1.0;
        n.xy *= u_normalScale;
        n.z = sqrt(saturate(1.0 - dot(n.xy, n.xy)));
        return n;
    }
    else
    {
        // the up vector (w) in tangent space is the default normal
        // that's why normal maps are mostly blue
        return float3(0.0, 0.0, 1.0);
    }
}

float materialOcclusionValue(float2 texcoord)
{
    if(u_hasOcclusionTexture)
    {
        // occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
        float occlusion = s_texOcclusion.Sample(s_texOcclusionState, texcoord).r;
        return occlusion + (1.0 - occlusion) * (1.0 - u_occlusionStrength);
    }
    else
    {
        return 1.0;
    }
}

float3 materialEmissiveValue(float2 texcoord)
{
    if(u_hasEmissiveTexture)
    {
        return s_texEmissive.Sample(s_texEmissiveState, texcoord).rgb * u_emissiveFactor;
    }
    else
    {
        return u_emissiveFactor;
    }
}

Material getMaterial(float2 texcoord)
{
    Material mat;

    // Read textures/uniforms

    mat.albedo = materialAlbedoValue(texcoord);
    float2 metallicRoughness = materialMetallicRoughnessValue(texcoord);
    mat.metallic  = metallicRoughness.r;
    mat.roughness = metallicRoughness.g;
    mat.normal = materialNormalValue(texcoord);
    mat.occlusion = materialOcclusionValue(texcoord);
    mat.emissive = materialEmissiveValue(texcoord);

    mat = initMaterial(mat);

    return mat;
}

// Account for multiple scattering across microfacets
// Computes a scaling factor for the BRDF

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
        float3 factorMetallic = float3(1.0,1.0,1.0) + mat.F0 * (1.0 / E.x - 1.0);

        // for dielectrics, fresnel determines the ratio between specular and diffuse energy
        // so the albedo depends on F as a variable
        // however, dielectrics in GLTF have a fixed F0 of 0.04 so we can do this with a second LUT
        float3 factorDielectric = float3(1.0 / E.y, 1.0 / E.y, 1.0 / E.y);

        return lerp(factorDielectric, factorMetallic, mat.metallic);
    }
    else return float3(1.0,1.0,1.0);
}

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
