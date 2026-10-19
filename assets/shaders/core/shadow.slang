#include <darmok.inc.slang>
#include <darmok_skinning.inc.slang>

// Alpha test for shadow casters with mask materials.
// Bindings and uniforms match the material includes so that
// Material::renderBind can be used to feed them.
[[vk::binding(0, 0)]]
uniform Texture2D s_texBaseColor : register(t0);
[[vk::binding(0, 0)]]
SamplerState s_texBaseColorState : register(s0);

uniform float4 u_baseColorFactor;
uniform float4 u_hasTextures;

struct VSInput
{
    float3 a_position : POSITION;
    float4 a_indices : BLENDINDICES;
    float4 a_weight : BLENDWEIGHT;
    float2 a_texcoord0 : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 v_texcoord0 : TEXCOORD0;
};

[shader("vertex")]
VSOutput vert(VSInput IN)
{
	float4 pos = float4(IN.a_position, 1.0);
	pos = applySkinning(pos, IN.a_indices, IN.a_weight);
    VSOutput OUT;
	OUT.position = mul(u_modelViewProj, pos);
    OUT.v_texcoord0 = IN.a_texcoord0;
    return OUT;
}

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

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
#ifdef DARMOK_VARIANT_ALPHA_MASK_ENABLED
    discardBelowAlphaCutoff(IN.v_texcoord0);
#endif
    return float4(0.0);
}
