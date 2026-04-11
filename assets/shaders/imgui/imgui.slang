
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
    float4 u_imageLodEnabled; // x = lod, y = enabled
}

VSOutput main(VSInput IN)
{
    VSOutput OUT;
    OUT.position = mul(u_modelViewProj, float4(IN.a_position.xy, 0.0, 1.0));
    OUT.v_texcoord0 = IN.a_texcoord0;
    OUT.v_color0 = IN.a_color0;
    return OUT;
}

Texture2D<float4> s_texColor;
SamplerState s_texColorState;

float4 main(VSOutput IN) : SV_Target
{
#if DARMOK_VARIANT_LOD_ENABLED
    float3 color = s_texColor.SampleLevel(s_texColor_s, IN.v_texcoord0, u_imageLodEnabled.x).xyz;
    float alpha = 0.2 + 0.8 * u_imageLodEnabled.y;
    return float4(color, alpha) * IN.v_color0;
#else
    float4 texel = s_texColor.Sample(s_texColor_s, IN.v_texcoord0);
    return texel * IN.v_color0;
#endif
}
