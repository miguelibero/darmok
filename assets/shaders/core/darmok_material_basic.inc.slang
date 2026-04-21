#pragma once

#include <darmok_sampler.inc.slang>

[[vk::binding(1, 0)]]
Texture2D<float4> s_texBaseColor : register(t1);
[[vk::binding(1, 0)]]
SamplerState s_texBaseColorState : register(s1);

[[vk::binding(2, 0)]]
Texture2D<float4> s_texSpecular : register(t2);
[[vk::binding(2, 0)]]
SamplerState s_texSpecularState : register(s2);

cbuffer MaterialUniforms
{
    float4 u_baseColorFactor;
    float4 u_specularFactorVec;
    float4 u_hasTextures;
}

inline bool getBitfieldValue(float4 field, int pos)
{
    return ((uint(field.x) & (1u << pos)) != 0u);
}

#define u_hasBaseColorTexture getBitfieldValue(u_hasTextures, 0)
#define u_hasSpecularTexture  getBitfieldValue(u_hasTextures, 1)

#define u_specularFactor (u_specularFactorVec.xyz)
#define u_shininess      (u_specularFactorVec.w)

struct Material
{
    float4 diffuse;
    float3 specular;
    float shininess;
};

float4 materialDiffuseValue(float2 texcoord)
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

float3 materialSpecularValue(float2 texcoord)
{
    if(u_hasSpecularTexture)
    {
        return s_texSpecular.Sample(s_texSpecularState, texcoord).rgb * u_specularFactor;
    }
    else
    {
        return u_specularFactor;
    }
}

Material getMaterial(float2 texcoord)
{
    Material mat;
    mat.diffuse = materialDiffuseValue(texcoord);
    mat.specular = materialSpecularValue(texcoord);
    mat.shininess = u_shininess;
    return mat;
}

float3 phongDiffuse(float3 lightDir, float3 normal, float3 radianceIn)
{
    float diff = max(dot(normal, lightDir), 0.0);
    return diff * radianceIn;
}

float3 phongSpecular(float3 viewDir, float3 lightDir, float3 normal, float3 radianceIn, float shininess)
{
    float3 reflectDir = reflect(-lightDir, normal);
    float NdotR = max(dot(viewDir, reflectDir), 0.0);
    float spec = pow(max(NdotR, 1e-8), max(shininess, 0.0));
    return spec * radianceIn;
}
