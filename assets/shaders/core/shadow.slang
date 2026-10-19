#include <darmok.inc.slang>
#include <darmok_skinning.inc.slang>
#include <darmok_alpha_mask.inc.slang>

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

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
#ifdef DARMOK_VARIANT_ALPHA_MASK_ENABLED
    discardBelowAlphaCutoff(IN.v_texcoord0);
#endif
    return float4(0.0);
}
