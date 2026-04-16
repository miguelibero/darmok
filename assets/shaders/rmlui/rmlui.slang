struct VSInput
{
    float2 a_position : POSITION;
    float2 a_texcoord0 : TEXCOORD0;
    float4 a_color0 : COLOR0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 v_texcoord0 : TEXCOORD0;
    float4 v_color0 : COLOR0;
};

cbuffer Constants
{
    float4x4 u_modelViewProj;
    float4 u_rmluiData;
}

#define u_rmluiForceDepth ((int(u_rmluiData.x) & 1 << 0) != 0)
#define u_rmluiDepth u_rmluiData.y

[shader("vertex")]
VSOutput vert(VSInput IN)
{
    VSOutput OUT;
    OUT.position = mul(u_modelViewProj, float4(IN.a_position.xy, 0.0, 1.0));
    if(u_rmluiForceDepth)
    {
        OUT.position.z = u_rmluiDepth;
    }
    OUT.v_color0 = IN.a_color0;
#ifndef DARMOK_VARIANT_TEXTURE_DISABLE
    OUT.v_texcoord0 = IN.a_texcoord0;
#endif
    return OUT;
}

#ifndef DARMOK_VARIANT_TEXTURE_DISABLE
Texture2D<float4> s_texColor : register(t1);
SamplerState s_texColorState : register(s1);
#endif

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
    float4 outColor = IN.v_color0;
#ifndef DARMOK_VARIANT_TEXTURE_DISABLE
    outColor *= s_texColor.Sample(s_texColorState, IN.v_texcoord0);
#endif
    return outColor;
}
