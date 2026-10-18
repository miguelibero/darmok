// Deferred lighting pass.
// Material texture slots 0-3 are unused in this pass; we reuse them for G-buffer.
// Light (10-12) and shadow (13-14) slots are shared with the forward renderer.

#include <darmok_brdf.inc.slang>
#include <darmok_light.inc.slang>
#include <darmok_shadow.inc.slang>
#include <darmok_pbr.inc.slang>
#include <darmok.inc.slang>

// G-buffer (reusing material slots 0-3, which are not needed in this pass)
[[vk::binding(0, 0)]]
uniform Texture2D s_gbufferAlbedoMetallic : register(t0);
[[vk::binding(0, 0)]]
SamplerState s_gbufferAlbedoMetallicState : register(s0);

[[vk::binding(1, 0)]]
uniform Texture2D s_gbufferNormalRoughnessOcclusion : register(t1);
[[vk::binding(1, 0)]]
SamplerState s_gbufferNormalRoughnessOcclusionState : register(s1);

[[vk::binding(2, 0)]]
uniform Texture2D s_gbufferEmissive : register(t2);
[[vk::binding(2, 0)]]
SamplerState s_gbufferEmissiveState : register(s2);

[[vk::binding(3, 0)]]
uniform Texture2D<float> s_gbufferDepth : register(t3);
[[vk::binding(3, 0)]]
SamplerState s_gbufferDepthState : register(s3);

// bgfx built-in: inverse view matrix (camera space -> world space)
uniform float4x4 u_invView;

// Set by LightingRenderComponent::beforeRenderEntity
uniform float4 u_camPos;

struct VSInput
{
    float3 a_position : POSITION;
    float2 a_texcoord0 : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 v_texcoord0 : TEXCOORD0;
};

float3 calcLightRadiance(LightData data, float3 V, float3 N, float NoV, Material mat)
{
    float NoL = saturate(dot(N, data.direction));
    return BRDF(V, data.direction, N, NoV, NoL, mat) * data.radiance * NoL;
}

[shader("vertex")]
VSOutput vert(VSInput IN)
{
    VSOutput OUT;
    OUT.position = float4(IN.a_position.xy, 0.0, 1.0);
    OUT.v_texcoord0 = IN.a_texcoord0;
    return OUT;
}

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
    float2 texCoord = IN.v_texcoord0;

    // Sample G-buffer
    float4 albedoMetallic = s_gbufferAlbedoMetallic.Sample(s_gbufferAlbedoMetallicState, texCoord);
    float4 normalRoughnessOcclusion = s_gbufferNormalRoughnessOcclusion.Sample(s_gbufferNormalRoughnessOcclusionState, texCoord);
    float4 emissiveData = s_gbufferEmissive.Sample(s_gbufferEmissiveState, texCoord);
    float depth = s_gbufferDepth.Sample(s_gbufferDepthState, texCoord).r;

    // Reconstruct world-space position from depth buffer
    float4 viewPos = screen2Eye(float4(IN.position.xy, depth, 1.0));
    float3 worldPos = mul(u_invView, viewPos).xyz;

    // Normals were stored in view space; rotate back to world space
    float3 viewN = unpackNormal(normalRoughnessOcclusion.rg);
    float3 N = normalize(mul(float3x3(u_invView), viewN));

    // Reconstruct Material
    Material mat;
    mat.albedo = float4(albedoMetallic.rgb, 1.0);
    mat.metallic = albedoMetallic.a;
    mat.roughness = normalRoughnessOcclusion.b;
    mat.occlusion = normalRoughnessOcclusion.a;
    mat.emissive = emissiveData.rgb;
    mat.normal = N;
    mat = initMaterial(mat);
    mat.a = specularAntiAliasing(N, mat.a);

    float3 V = u_camPos.xyz - worldPos;

    return applyFragmentLights(V, N, worldPos, mat);
}
