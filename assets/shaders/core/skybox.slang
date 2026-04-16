
cbuffer Constants
{
    float4x4 u_modelViewProj;
}

struct VSInput
{
    float3 a_position : POSITION;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 v_direction : TEXCOORD0;
};

Texture2D<float4> s_texSkybox;
SamplerState s_texSkyboxState;

[shader("vertex")]
VSOutput vert(VSInput IN)
{
    VSOutput OUT;
    OUT.position = mul(u_modelViewProj, float4(IN.a_position,1.0));
    OUT.v_direction = IN.a_position;
    return OUT;
}

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
    return s_texSkybox.Sample(s_texSkyboxState, IN.v_direction.xy);
}

