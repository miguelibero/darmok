#include <darmok_skinning.inc.slang>
#include <darmok_material_basic.inc.slang>
#include <darmok.inc.slang>

uniform float4x4 u_modelViewProj;
uniform Texture2D s_texColor;
SamplerState samplerState;

struct VSInput
{
    float3 a_position  : POSITION;
    [DarmokVertexNormalize]
    float4 a_color0    : COLOR0;
    uint4 a_indices    : BLENDINDICES;
    float4 a_weight    : BLENDWEIGHT;
    float2 a_texcoord0 : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 v_color0 : COLOR0;
    float2 v_texcoord0 : TEXCOORD0;
};

[shader("vertex")]
VSOutput vert(VSInput IN)
{
    VSOutput OUT;
    float4 pos = float4(IN.a_position, 1.0);
    pos = applySkinning(pos, IN.a_indices, IN.a_weight);
    OUT.position = mul(u_modelViewProj, pos);
    OUT.v_color0 = IN.a_color0;
    OUT.v_texcoord0 = IN.a_texcoord0;
    return OUT;
}

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
	Material mat = getMaterial(IN.v_texcoord0);
	return mat.diffuse * IN.v_color0;
}
