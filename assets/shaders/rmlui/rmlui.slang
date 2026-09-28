#include <darmok.inc.slang>

struct VSInput
{
    float2 a_position : POSITION;

    [DarmokVertexNormalize]
    float4 a_color0 : COLOR0;

    float2 a_texcoord0 : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 v_texcoord0 : TEXCOORD0;
    float4 v_color0 : COLOR0;
    float2 v_docPosition : TEXCOORD1;
};

uniform float4x4 u_modelViewProj;
uniform float4 u_rmluiData;

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
    OUT.v_docPosition = IN.a_position.xy;
#ifndef DARMOK_VARIANT_TEXTURE_DISABLE
    OUT.v_texcoord0 = IN.a_texcoord0;
#endif
    return OUT;
}

#if !defined(DARMOK_VARIANT_TEXTURE_DISABLE) || defined(DARMOK_VARIANT_GRADIENT)
uniform Texture2D s_texColor;
SamplerState s_texColorState;
#endif

#ifdef DARMOK_VARIANT_GRADIENT
uniform float4 u_rmluiGradient; // p0.xy, p1.xy
#endif

#ifdef DARMOK_VARIANT_MASK
uniform Texture2D s_texMask;
SamplerState s_texMaskState;
#endif

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
#ifdef DARMOK_VARIANT_GRADIENT
    float2 p0 = u_rmluiGradient.xy;
    float2 p1 = u_rmluiGradient.zw;
    float2 dir = p1 - p0;
    float len2 = dot(dir, dir);
    float t = 0.0;
    if (len2 > 0.0001)
    {
        t = clamp(dot(IN.v_docPosition - p0, dir) / len2, 0.0, 1.0);
    }
    return s_texColor.Sample(s_texColorState, float2(t, 0.5));
#else
    float4 outColor = IN.v_color0;
#ifndef DARMOK_VARIANT_TEXTURE_DISABLE
    outColor *= s_texColor.Sample(s_texColorState, IN.v_texcoord0);
#endif
#ifdef DARMOK_VARIANT_MASK
    float4 maskColor = s_texMask.Sample(s_texMaskState, IN.v_texcoord0);
    outColor.a *= maskColor.a;
#endif
    return outColor;
#endif
}
