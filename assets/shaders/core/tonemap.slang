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

uniform Texture2D s_texColor;
SamplerState s_texColorState;

// Linear RGB -> sRGB display encoding.
float3 linearToSRGB(float3 color)
{
    color = max(color, float3(0.0));

    float3 low = color * 12.92;
    float3 high =
        1.055 * pow(color, float3(1.0 / 2.4)) - 0.055;

    return lerp(
        low,
        high,
        step(float3(0.0031308), color)
    );
}

[shader("vertex")]
VSOutput vert(VSInput IN)
{
    VSOutput OUT;
    OUT.position = mul(
        u_modelViewProj,
        float4(IN.a_position, 0.0, 1.0)
    );
    OUT.v_texcoord0 = IN.a_texcoord0;
    return OUT;
}

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
    float4 baseColor =
        s_texColor.Sample(s_texColorState, IN.v_texcoord0);

    // Exposure adjustment; lower this to darken the image.
    const float exposure = 0.5;
    float3 hdrColor = max(baseColor.rgb, float3(0.0)) * exposure;

    // Reinhard tone mapping.
    float3 mapped = hdrColor / (float3(1.0) + hdrColor);

    // Convert linear output to display sRGB.
    mapped = linearToSRGB(mapped);

    return float4(mapped, baseColor.a);
}