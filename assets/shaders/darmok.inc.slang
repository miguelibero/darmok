#pragma once

uniform float4 u_viewRect;
uniform float4x4 u_invProj;

[__AttributeUsage(_AttributeTargets.Var)]
struct DarmokVertexNormalizeAttribute
{
};

[__AttributeUsage(_AttributeTargets.Var)]
struct DarmokVertexAsIntAttribute
{
};

#ifdef DARMOK_GLSL
// invert the matrix multiply in glsl because bgfx works that way
#define mul(M, v) ::mul(v, M)
#endif

inline bool getBitfieldValue(float4 field, int pos)
{
    return ((uint(field.x) & (1u << pos)) != 0u);
}

// from screen coordinates (gl_FragCoord) to eye space
float4 screen2Eye(float4 coord)
{
#ifdef DARMOK_GLSL
    // https://www.khronos.org/opengl/wiki/Compute_eye_space_from_window_space
    float3 ndc = float3(
        2.0 * (coord.x - u_viewRect.x) / u_viewRect.z - 1.0,
        2.0 * (coord.y - u_viewRect.y) / u_viewRect.w - 1.0,
        2.0 * coord.z - 1.0
    );
#else
    float3 ndc = float3(
        2.0 * (coord.x - u_viewRect.x) / u_viewRect.z - 1.0,
        2.0 * (u_viewRect.w - coord.y - 1 - u_viewRect.y) / u_viewRect.w - 1.0,
        coord.z
    );
#endif

    // https://stackoverflow.com/a/16597492/862300
    float4 eye = mul(u_invProj, float4(ndc, 1.0));
    eye = eye / eye.w;
    return eye;
}

// depth from screen coordinates (gl_FragCoord.z) to eye space
// same as screen2Eye(vec4(0, 0, depth, 1)).z but slightly faster
// (!) this assumes a perspective projection as created by bx::mtxProj
// for a left-handed coordinate system
float screen2EyeDepth(float depth, float near, float far)
{
#ifdef DARMOK_GLSL
    float ndc = 2.0 * depth - 1.0;
    // ndc = (eye * (far + near) / (far - near) - 2 * (far * near) / (far - near)) / eye
    float eye = 2.0 * far * near / (far + near + ndc * (near - far));
#else
    float ndc = depth;
    // ndc = (eye * far / (far - near) - (far * near) / (far - near)) / eye
    float eye = far * near / (far + ndc * (near - far));
#endif
    return eye;
}

// convert normal from tangent space to space of normal_ref and tangent_ref
float3 convertTangentNormal(float3 normal_ref, float3 tangent_ref, float3 normal)
{
    float3 bitangent = cross(normal_ref, tangent_ref);
    float3x3 TBN = float3x3(normalize(tangent_ref), normalize(bitangent), normalize(normal_ref));
    return normalize(mul(TBN, normal));
}

// compress viewspace normal into two components
// spheremap transform used by Cry Engine 3
// https://aras-p.info/texts/CompactNormalStorage.html#method04spheremap
// must be viewspace so we can hide z-pole
// both (0,0,1) and (0,0,-1) have the same encoding
// the original implementation always decoded to (0,0,1)
// changed it to return normals pointing towards the camera for
// a left-handed coordinate system

float2 packNormal(float3 normal)
{
    float f = sqrt(8.0 * -normal.z + 8.0);
    return normal.xy / f + 0.5;
}

float3 unpackNormal(float2 encoded)
{
    float2 fenc = encoded * 4.0 - 2.0;
    float f = dot(fenc, fenc);
    float g = sqrt(1.0 - f * 0.25);
    return float3(fenc * g, -(1.0 - f * 0.5));
}
