
float4x4 u_modelViewProj;

struct VSInput
{
    float2 a_position : POSITION;
    float2 a_texcoord0 : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 v_texcoord0 : TEXCOORD0;
};

Texture2D s_texColor;
SamplerState s_texColorState;

[shader("vertex")]
VSOutput vert(VSInput IN)
{
    VSOutput OUT;
    OUT.position = mul(u_modelViewProj, float4(IN.a_position,0.0,1.0));
    OUT.v_texcoord0 = IN.a_texcoord0;
    return OUT;
}

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
    float4 baseColor = s_texColor.Sample(s_texColorState, IN.v_texcoord0);
    float3 hdrColor = baseColor.rgb;
  
    // reinhard tone mapping
    float3 mapped = hdrColor / (hdrColor + float3(1.0));
    
    // gamma correction
    const float gamma = 2.2;
    mapped = pow(abs(mapped), float3(1.0 / gamma));
  
    return float4(mapped, baseColor.a);
}
