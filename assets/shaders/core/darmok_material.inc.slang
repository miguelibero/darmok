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

uniform float4 u_baseColorFactor;
uniform float4 u_metallicRoughnessNormalOcclusionFactor;
uniform float4 u_emissiveFactorVec;
uniform float4 u_hasTextures;

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