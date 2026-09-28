#include <darmok_skinning.inc.slang>
#include <darmok_material.inc.slang>
#include <darmok.inc.slang>

struct VSInput
{
    float3 a_position  : POSITION;
    float3 a_normal    : NORMAL;
    float3 a_tangent   : TANGENT;
    float4 a_indices   : BLENDINDICES;
    float4 a_weight    : BLENDWEIGHT;
    float2 a_texcoord0 : TEXCOORD0;
};

struct VSOutput
{
    float4 position    : SV_Position;
    float3 v_normal    : NORMAL;
    float3 v_tangent   : TANGENT;
    float2 v_texcoord0 : TEXCOORD0;
};

struct PSOutput
{
    float4 rt0 : SV_Target0;  // albedo.rgb + metallic
    float4 rt1 : SV_Target1;  // packNormal(viewN).xy + roughness + occlusion
    float4 rt2 : SV_Target2;  // emissive.rgb
};

uniform float3x3 u_normalMatrix;
uniform float4x4 u_model[32];
uniform float4x4 u_viewProj;
uniform float4x4 u_view;

[shader("vertex")]
VSOutput vert(VSInput IN)
{
    VSOutput OUT;
    float4 pos     = float4(IN.a_position, 1.0);
    float4 norm    = float4(IN.a_normal, 0.0);
    float4 tangent = float4(IN.a_tangent, 0.0);
    pos     = applySkinning(pos,     IN.a_indices, IN.a_weight);
    norm    = applySkinning(norm,    IN.a_indices, IN.a_weight);
    tangent = applySkinning(tangent, IN.a_indices, IN.a_weight);
    pos = mul(u_model[0], pos);

    OUT.position    = mul(u_viewProj, pos);
    float3 N = normalize(mul(u_normalMatrix, norm.xyz));
    float3 T = normalize(mul(u_normalMatrix, tangent.xyz));
    // Gram-Schmidt
    T = normalize(T - N * dot(N, T));
    OUT.v_normal    = N;
    OUT.v_tangent   = T;
    OUT.v_texcoord0 = IN.a_texcoord0;
    return OUT;
}

[shader("fragment")]
PSOutput frag(VSOutput IN)
{
    PSOutput OUT;
    Material mat = getMaterial(IN.v_texcoord0);

    // Apply normal map: tangent space -> world space
    float3 N = convertTangentNormal(IN.v_normal, IN.v_tangent, mat.normal);
    mat.a = specularAntiAliasing(N, mat.a);

    // Transform world-space normal to view space for compact storage.
    // packNormal encodes the hemisphere facing the camera correctly in view space.
    float3 viewN = normalize(mul(float3x3(u_view), N));

    OUT.rt0 = float4(mat.albedo.rgb, mat.metallic);
    OUT.rt1 = float4(packNormal(viewN), mat.roughness, mat.occlusion);
    OUT.rt2 = float4(mat.emissive, 0.0);
    return OUT;
}
