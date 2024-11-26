$input v_nearPoint, v_farPoint, v_texcoord0

uniform vec4 u_gridColor1;
uniform vec4 u_gridColor2;
uniform vec4 u_xAxisColor;
uniform vec4 u_zAxisColor;
uniform vec4 u_data;

#define u_nearPlane u_data.x
#define u_farPlane u_data.y
#define u_gridScale1 u_data.z
#define u_gridScale2 u_data.w
#define u_fadeFactor 0.9

#include <bgfx_shader.sh>
#include <darmok_util.sc>

// http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
// https://bgolus.medium.com/the-best-darn-grid-shader-yet-727f9278b9d8

float pristineGrid(vec2 uv, float scale, float lineWidth)
{
    uv /= scale;
    lineWidth = saturate(lineWidth / scale);
    vec2 duv = fwidth(uv);
    bool invertLine = lineWidth > 0.5;
    float targetWidth = invertLine ? 1.0 - lineWidth : lineWidth;
    vec2 targetWidth2 = vec2_splat(targetWidth);
    vec2 drawWidth = clamp(targetWidth2, duv, vec2_splat(0.5));
    vec2 lineAA = max(duv, 0.000001) * 1.5;
    vec2 gridUV = abs(fract(uv) * 2.0 - 1.0);
    gridUV = invertLine ? gridUV : 1.0 - gridUV;
    vec2 grid2 = smoothstep(drawWidth + lineAA, drawWidth - lineAA, gridUV);
    grid2 *= saturate(targetWidth2 / drawWidth);
    grid2 = mix(grid2, targetWidth2, saturate(duv * 2.0 - 1.0));
    grid2 = invertLine ? 1.0 - grid2 : grid2;
    return mix(grid2.x, 1.0, grid2.y);
}

float basicGrid(vec2 uv, float scale)
{
    uv /= scale;
    vec2 duv = fwidth(uv);
    vec2 v = abs(fract(uv - 0.5) - 0.5) / duv;
    return 1.0 - min(min(v.x, v.y), 1.0);
}

float grid(vec2 uv, float scale)
{
    return pristineGrid(uv, scale, 0.01);
}

float computeDepth(vec3 pos)
{
    vec4 p = mul(u_viewProj, vec4(pos.xyz, 1.0));
    return (p.z / p.w);
}

void main()
{
    float t = -v_nearPoint.y / (v_farPoint.y - v_nearPoint.y);
    vec3 pos = v_nearPoint + t * (v_farPoint - v_nearPoint);
    float ft = float(t > 0.0);
    float alpha1 = ft * grid(pos.xz, u_gridScale1);
    float alpha2 = ft * grid(pos.xz, u_gridScale2);
    vec4 color1 = vec4(u_gridColor1.xyz, u_gridColor1.w * alpha1);
    vec4 color2 = vec4(u_gridColor2.xyz, u_gridColor2.w * alpha2);
    gl_FragColor = mix(color1, color2, 0.5);

    float depth = computeDepth(pos);
    float eyeDepth = screen2EyeDepth(depth, u_nearPlane, u_farPlane);
    float linDepth = eyeDepth / u_farPlane;
    float fading = max(0, (u_fadeFactor - linDepth));
    gl_FragColor.a *= fading;
    gl_FragDepth = gl_FragColor.a > 0.001 ? depth : 1.0;
}