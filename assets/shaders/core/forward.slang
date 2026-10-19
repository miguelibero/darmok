#include <darmok.inc.slang>
#include <darmok_skinning.inc.slang>
#include <darmok_pbr.inc.slang>

struct VSInput
{
    float3 a_position  : POSITION;
    float3 a_normal    : NORMAL;
    float3 a_tangent   : TANGENT;
    float4 a_indices    : BLENDINDICES;
    float4 a_weight    : BLENDWEIGHT;
    float2 a_texcoord0 : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 v_position: POSITION;
    float3 v_normal : NORMAL;
    float3 v_tangent : TANGENT;
    float2 v_texcoord0 : TEXCOORD0;
    float3 v_viewDir : VIEWDIR;
};

[shader("vertex")]
VSOutput vert(VSInput IN)
{
    VSOutput OUT;
    float4 pos = float4(IN.a_position, 1.0);
    float4 norm = float4(IN.a_normal, 0.0);
    float4 tangent = float4(IN.a_tangent, 0.0);
    pos = applySkinning(pos, IN.a_indices, IN.a_weight);
    norm = applySkinning(norm, IN.a_indices, IN.a_weight);
    tangent = applySkinning(tangent, IN.a_indices, IN.a_weight);
    pos = mul(u_model[0], pos);

    OUT.v_position = pos.xyz;    
    OUT.v_viewDir = normalize(u_camPos.xyz - OUT.v_position);
    OUT.v_texcoord0 = IN.a_texcoord0;
    OUT.position = mul(u_viewProj, pos);

    float3 N = normalize(mul(u_normalMatrix, norm.xyz));
    float3 T = normalize(mul(u_normalMatrix, tangent.xyz));
    // Gram-Schmidt
    T = normalize(T - N * dot(N, T));
    OUT.v_normal = N;
    OUT.v_tangent = T;

    return OUT;
}

#ifdef DARMOK_VARIANT_SCENE_DEPTH_ENABLED

[[vk::binding(15, 0)]]
uniform Texture2D<float> s_sceneDepth : register(t15);

#endif

void discardIfHiddenBySceneDepth(float4 fragCoord)
{
#ifdef DARMOK_VARIANT_SCENE_DEPTH_ENABLED
    float sceneDepth = s_sceneDepth.Load(int3(int2(fragCoord.xy), 0));
    if (fragCoord.z > sceneDepth)
    {
        discard;
    }
#endif
}


[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
    discardIfHiddenBySceneDepth(IN.position);
    Material mat = getMaterial(IN.v_texcoord0);
    discardBelowAlphaCutoff(mat.albedo.a);
    // convert normal map from tangent space -> world space (= space of tangent, etc.)
    float3 N = convertTangentNormal(IN.v_normal, IN.v_tangent, mat.normal);
    return applyFragmentLights(IN.v_viewDir, N, IN.v_position, mat);
}
