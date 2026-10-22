#include <darmok.inc.slang>

struct VSInput
{
    float3 a_position : POSITION;
};

struct VSOutput
{
    float4 position : SV_Position;
};

struct PSOutput
{
    float4 color : SV_Target0;
    float depth : SV_Depth;
};

float4 u_minorGridColor;
float4 u_majorGridColor;
float4 u_axisXColor;
float4 u_axisZColor;

float4 u_gridParams;
float4 u_depthParams;
float4 u_fadeParams;

#define u_minorSpacing  u_gridParams.x
#define u_majorSpacing  u_gridParams.y
#define u_minorWidth    u_gridParams.z
#define u_majorWidth    u_gridParams.w

#define u_nearPlane     u_depthParams.x
#define u_farPlane      u_depthParams.y
#define u_depthEpsilon  u_depthParams.z

#define u_fadeStart     u_fadeParams.x
#define u_fadeEnd       u_fadeParams.y
#define u_axisWidth     u_fadeParams.z

void discardIfHiddenBySceneDepth(float3 fragCoord);

[shader("vertex")]
VSOutput vs_main(VSInput input)
{
    VSOutput output;
    output.position = float4(input.a_position.xy, 0.0, 1.0);
    return output;
}

float3 getCameraPosition()
{
    return mul(
        u_invView,
        float4(0.0, 0.0, 0.0, 1.0)
    ).xyz;
}

float3 reconstructWorldRay(float2 pixelPosition)
{
    float2 uv = (pixelPosition - u_viewRect.xy) / u_viewRect.zw;

    float2 ndc = float2(
        uv.x * 2.0 - 1.0,
        1.0 - uv.y * 2.0
    );

    float4 farWorld = mul(
        u_invViewProj,
        float4(ndc, 1.0, 1.0)
    );

    farWorld.xyz /= farWorld.w;
    float3 cameraPosition = getCameraPosition();
    return normalize(farWorld.xyz - cameraPosition);
}

float gridLine(float2 coord, float spacing, float width)
{
    float2 derivative = max(
        fwidth(coord),
        float2(1e-6, 1e-6)
    );

    float2 distanceToLine =
        abs(frac(coord / spacing + 0.5) - 0.5) * spacing;

    float2 line = 1.0 - smoothstep(
        width * 0.5 - derivative,
        width * 0.5 + derivative,
        distanceToLine
    );

    return max(line.x, line.y);
}

[shader("fragment")]
PSOutput fs_main(VSOutput input)
{
    PSOutput output;

    float3 cameraPosition = getCameraPosition();
    float3 rayDirection = reconstructWorldRay(input.position.xy);

    if (abs(rayDirection.y) < 1e-6)
    {
        discard;
    }

    float t = -cameraPosition.y / rayDirection.y;

    if (t <= 0.0)
    {
        discard;
    }

    float3 worldPos = cameraPosition + rayDirection * t;

    float4 gridClip = mul(
        u_viewProj,
        float4(worldPos, 1.0)
    );

    if (gridClip.w <= 0.0)
        discard;

    float gridDepth = gridClip.z / gridClip.w;

    discardIfHiddenBySceneDepth(
        float3(input.position.xy, gridDepth)
    );

    float spacing = max(u_minorSpacing, 1e-5);
    float majorInterval = max(u_majorSpacing, 1.0);

    float minor = gridLine(
        worldPos.xz,
        spacing,
        u_minorWidth
    );

    float major = gridLine(
        worldPos.xz,
        spacing * majorInterval,
        u_majorWidth
    );

    float axisX = 1.0 - smoothstep(
        u_axisWidth,
        u_axisWidth + max(fwidth(worldPos.z), 1e-6),
        abs(worldPos.z)
    );

    float axisZ = 1.0 - smoothstep(
        u_axisWidth,
        u_axisWidth + max(fwidth(worldPos.x), 1e-6),
        abs(worldPos.x)
    );

    float3 color = u_minorGridColor.rgb;
    float alpha = minor * u_minorGridColor.a;

    color = lerp(color, u_majorGridColor.rgb, major);
    alpha = max(alpha, major * u_majorGridColor.a);

    color = lerp(color, u_axisXColor.rgb, axisX);
    color = lerp(color, u_axisZColor.rgb, axisZ);

    alpha = max(alpha, axisX * u_axisXColor.a);
    alpha = max(alpha, axisZ * u_axisZColor.a);

    float distanceToCamera = length(worldPos - cameraPosition);

    float fade = 1.0 - smoothstep(
        u_fadeStart,
        max(u_fadeEnd, u_fadeStart + 1e-5),
        distanceToCamera
    );

    alpha *= fade;

    if (alpha < 0.001)
    {
        discard;
    }

    output.color = float4(color, alpha);
    output.depth = gridDepth;

    return output;
}
