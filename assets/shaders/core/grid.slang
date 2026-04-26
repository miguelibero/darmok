#include <darmok_util.inc.slang>

float4x4 u_invViewProj;
float4x4 u_viewProj;

float4 u_gridColor1;
float4 u_gridColor2;
float4 u_xAxisColor;
float4 u_zAxisColor;
float4 u_data;

struct VSInput
{
    float2 a_position : POSITION;
};

struct VSOutput
{
    float4 position: SV_Position;
    float3 v_nearPoint: POSITION0;
    float3 v_farPoint: POSITION1;
    float2 v_texcoord0 : TEXCOORD0;
};

float3 unproject(float3 pos)
{
    float4 p = mul(u_invViewProj, float4(pos, 1.0));
    return p.xyz / p.w;
}

[shader("vertex")]
VSOutput vert(VSInput IN)
{
	float4 pos = float4(IN.a_position, 0.0, 1.0);
    VSOutput OUT;
	OUT.v_nearPoint = unproject(float3(IN.a_position, 0.0));
    OUT.v_farPoint = unproject(float3(IN.a_position, 1.0));
	OUT.v_texcoord0 = IN.a_position;
	OUT.position = pos;
    return OUT;
}

#define u_nearPlane u_data.x
#define u_farPlane u_data.y
#define u_gridScale1 u_data.z
#define u_gridScale2 u_data.w
#define u_fadeFactor 0.9

// http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
// https://bgolus.medium.com/the-best-darn-grid-shader-yet-727f9278b9d8

float pristineGrid(float2 uv, float scale, float lineWidth)
{
    uv /= scale;
    lineWidth = saturate(lineWidth / scale);
    float2 duv = abs(ddx(uv)); // approximate fwidth
    bool invertLine = lineWidth > 0.5;
    float targetWidth = invertLine ? 1.0 - lineWidth : lineWidth;
    float2 targetWidth2 = float2(targetWidth, targetWidth);
    float2 drawWidth = clamp(targetWidth2, duv, float2(0.5,0.5));
    float2 lineAA = max(duv, float2(0.000001,0.000001)) * 1.5;
    float2 gridUV = abs(frac(uv) * 2.0 - 1.0);
    gridUV = invertLine ? gridUV : 1.0 - gridUV;
    float2 grid2 = smoothstep(drawWidth + lineAA, drawWidth - lineAA, gridUV);
    grid2 *= saturate(targetWidth2 / drawWidth);
    grid2 = lerp(grid2, targetWidth2, saturate(duv * 2.0 - 1.0));
    grid2 = invertLine ? 1.0 - grid2 : grid2;
    return lerp(grid2.x, 1.0, grid2.y);
}

float grid(float2 uv, float scale)
{
    return pristineGrid(uv, scale, 0.01);
}

float computeDepth(float3 pos)
{
    float4 p = mul(u_viewProj, float4(pos.xyz, 1.0));
    return (p.z / p.w);
}

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
    float t = -IN.v_nearPoint.y / (IN.v_farPoint.y - IN.v_nearPoint.y);
    float3 pos = IN.v_nearPoint + t * (IN.v_farPoint - IN.v_nearPoint);
    float ft = float(t > 0.0);
    float alpha1 = ft * grid(pos.xz, u_gridScale1);
    float alpha2 = ft * grid(pos.xz, u_gridScale2);
    float4 color1 = float4(u_gridColor1.xyz, u_gridColor1.w * alpha1);
    float4 color2 = float4(u_gridColor2.xyz, u_gridColor2.w * alpha2);
    float4 outColor = color1 + color2;

    float depth = computeDepth(pos);
    float eyeDepth = screen2EyeDepth(depth, u_nearPlane, u_farPlane);
    float linDepth = eyeDepth / u_farPlane;
    float fading = max(0, (u_fadeFactor - linDepth));
    outColor.a *= fading;
    // gl_FragDepth equivalent not directly set here
    return outColor;
}
