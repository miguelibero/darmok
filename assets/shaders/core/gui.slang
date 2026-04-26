float4x4 u_modelViewProj;
float4 u_color;

Texture2D s_texColor;
SamplerState samplerState;

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

[shader("vertex")]
VSOutput vert(VSInput IN)
{
    VSOutput OUT;
    OUT.position = mul(u_modelViewProj, float4(IN.a_position.xy, 0.0, 1.0));
    OUT.v_texcoord0 = IN.a_texcoord0;
    OUT.v_color0 = IN.a_color0;
    return OUT;
}

[shader("fragment")]
float4 frag(VSOutput input) : SV_Target
{
    float4 texColor = s_texColor.Sample(samplerState, input.v_texcoord0);
    return texColor * input.v_color0;
}