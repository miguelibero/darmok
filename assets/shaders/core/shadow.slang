
#include <darmok_skinning.inc.slang>

struct VSInput
{
    float3 a_position : POSITION;
    float4 a_indices : BLENDINDICES;
    float4 a_weight : BLENDWEIGHT;
};

struct VSOutput
{
    float4 position : SV_Position;
};

uniform float4x4 u_modelViewProj;

[shader("vertex")]
VSOutput vert(VSInput IN)
{
	float4 pos = float4(IN.a_position, 1.0);
	pos = applySkinning(pos, IN.a_indices, IN.a_weight);
    VSOutput OUT;
	OUT.position = mul(u_modelViewProj, pos);
    return OUT;
}

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
    return float4(0.0);
}