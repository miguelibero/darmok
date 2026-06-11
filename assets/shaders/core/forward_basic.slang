#include <darmok_skinning.inc.slang>
#include <darmok_forward_basic.inc.slang>

struct VSInput
{
    float3 a_position : POSITION;
    float3 a_normal : NORMAL;
    uint4 a_indices : BLENDINDICES;
    float4 a_weight : BLENDWEIGHT;
    float2 a_texcoord0 : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 v_position : POSITION;
    float3 v_normal : NORMAL;
    float2 v_texcoord0 : TEXCOORD0;
    float3 v_viewDir : VIEWDIR;
};

uniform float4 u_camPos;
uniform float3x3 u_normalMatrix;
uniform float4x4 u_model[32];
uniform float4x4 u_viewProj;

[shader("vertex")]
VSOutput vert(VSInput IN)
{
    VSOutput OUT;
    float4 pos = float4(IN.a_position, 1.0);
    float4 norm = float4(IN.a_normal, 0.0);
    pos = applySkinning(pos, IN.a_indices, IN.a_weight);
    norm = applySkinning(norm, IN.a_indices, IN.a_weight);
    pos = mul(u_model[0], pos);

    OUT.v_position = pos.xyz;
    OUT.v_normal = mul(u_normalMatrix, norm.xyz);
    OUT.v_viewDir = u_camPos.xyz - OUT.v_position;
    OUT.v_texcoord0 = IN.a_texcoord0;
    OUT.position = mul(u_viewProj, pos);
    return OUT;
}

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
    return forwardFragment(IN.v_viewDir, IN.v_normal, IN.v_position, IN.v_texcoord0);
}
