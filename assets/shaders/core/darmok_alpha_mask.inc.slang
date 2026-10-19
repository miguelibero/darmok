#pragma once

// Alpha test for shadow casters with mask materials.
// Bindings and uniforms match the material includes so that
// Material::renderBind can be used to feed them.
[[vk::binding(0, 0)]]
uniform Texture2D s_texBaseColor : register(t0);
[[vk::binding(0, 0)]]
SamplerState s_texBaseColorState : register(s0);

uniform float4 u_baseColorFactor;
uniform float4 u_hasTextures;

// u_hasTextures.y holds the alpha cutoff (0 = disabled)
void discardBelowAlphaCutoff(float2 texcoord)
{
    float cutoff = u_hasTextures.y;
    if (cutoff <= 0.0)
    {
        return;
    }
    float alpha = u_baseColorFactor.a;
    if ((uint(u_hasTextures.x) & 1u) != 0u)
    {
        alpha *= s_texBaseColor.Sample(s_texBaseColorState, texcoord).a;
    }
    if (alpha < cutoff)
    {
        discard;
    }
}
