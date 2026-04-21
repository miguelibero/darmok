#pragma once


[[vk::binding(0, 0)]]
Texture2D<float4> s_texAlbedoLUT : register(t0);
[[vk::binding(0, 0)]]
SamplerState s_texAlbedoLUTState : register(s0);

[[vk::binding(1, 0)]]
Texture2D<float4> s_texBaseColor : register(t1);
[[vk::binding(1, 0)]]
SamplerState s_texBaseColorState : register(s1);

[[vk::binding(3, 0)]]
Texture2D<float4> s_texMetallicRoughness : register(t3);
[[vk::binding(3, 0)]]
SamplerState s_texMetallicRoughnessState : register(s3);

[[vk::binding(4, 0)]]
Texture2D<float4> s_texNormal : register(t4);
[[vk::binding(4, 0)]]
SamplerState s_texNormalState : register(s4);

[[vk::binding(5, 0)]]
Texture2D<float4> s_texOcclusion : register(t5);
[[vk::binding(5, 0)]]
SamplerState s_texOcclusionState : register(s5);

[[vk::binding(6, 0)]]
Texture2D<float4> s_texEmissive : register(t6);
[[vk::binding(6, 0)]]
SamplerState s_texEmissiveState : register(s6);

cbuffer MaterialConstants
{
    float4 u_baseColorFactor;
    float4 u_metallicRoughnessNormalOcclusionFactor;
    float4 u_emissiveFactorVec;
    float4 u_hasTextures;
    float4 u_multipleScatteringVec;
}

#define u_hasBaseColorTexture         ((uint(u_hasTextures.x) & (1 << 0)) != 0)
#define u_hasMetallicRoughnessTexture ((uint(u_hasTextures.x) & (1 << 2)) != 0)
#define u_hasNormalTexture            ((uint(u_hasTextures.x) & (1 << 3)) != 0)
#define u_hasOcclusionTexture         ((uint(u_hasTextures.x) & (1 << 4)) != 0)
#define u_hasEmissiveTexture          ((uint(u_hasTextures.x) & (1 << 5)) != 0)

#define u_metallicRoughnessFactor (u_metallicRoughnessNormalOcclusionFactor.xy)
#define u_normalScale             (u_metallicRoughnessNormalOcclusionFactor.z)
#define u_occlusionStrength       (u_metallicRoughnessNormalOcclusionFactor.w)
#define u_emissiveFactor          (u_emissiveFactorVec.xyz)

#define u_multipleScattering   (u_multipleScatteringVec.x != 0.0)
#define u_whiteFurnaceRadiance (u_multipleScatteringVec.y)
#define u_whiteFurnace         (u_whiteFurnaceRadiance > 0.0)

#define PI     (3.14159265359)
#define INV_PI (0.31830988618)

struct Material
{
    float4 albedo;
    float metallic;
    float roughness;
    float3 normal;
    float occlusion;
    float3 emissive;

    // calculated from the above

    float3 diffuse; // this becomes black for higher metalness
    float3 F0; // Fresnel reflectance at normal incidence
    float a; // remapped roughness (^2)
};

float4 materialAlbedoValue(float2 texcoord)
{
    if(u_hasBaseColorTexture)
    {
        return s_texBaseColor.Sample(s_texBaseColorState, texcoord) * u_baseColorFactor;
    }
    else
    {
        return u_baseColorFactor;
    }
}

float2 materialMetallicRoughnessValue(float2 texcoord)
{
    if(u_hasMetallicRoughnessTexture)
    {
        float4 v = s_texMetallicRoughness.Sample(s_texMetallicRoughnessState, texcoord);
        return float2(v.b, v.g) * u_metallicRoughnessFactor;
    }
    else
    {
        return u_metallicRoughnessFactor;
    }
}

float3 materialNormalValue(float2 texcoord)
{
    if(u_hasNormalTexture)
    {
        // the normal scale can cause problems and serves no real purpose
        // normal compression and BRDF calculations assume unit length
        float3 n = s_texNormal.Sample(s_texNormalState, texcoord).rgb * 2.0 - 1.0;
        return normalize(n) * u_normalScale;
    }
    else
    {
        // the up vector (w) in tangent space is the default normal
        // that's why normal maps are mostly blue
        return float3(0.0, 0.0, 1.0);
    }
}

float materialOcclusionValue(float2 texcoord)
{
    if(u_hasOcclusionTexture)
    {
        // occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
        float occlusion = s_texOcclusion.Sample(s_texOcclusionState, texcoord).r;
        return occlusion + (1.0 - occlusion) * (1.0 - u_occlusionStrength);
    }
    else
    {
        return 1.0;
    }
}

float3 materialEmissiveValue(float2 texcoord)
{
    if(u_hasEmissiveTexture)
    {
        return s_texEmissive.Sample(s_texEmissiveState, texcoord).rgb * u_emissiveFactor;
    }
    else
    {
        return u_emissiveFactor;
    }
}

Material initMaterial(Material mat)
{
    // Taken directly from GLTF 2.0 specs
    // this can be precalculated instead of evaluating it in the BRDF for every light

    const float3 dielectricSpecular = float3(0.04, 0.04, 0.04);
    const float3 black = float3(0.0, 0.0, 0.0);

    // metals have no diffuse reflection so the albedo value stores F0 (reflectance at normal incidence) instead
    // dielectrics are assumed to have F0 = 0.04 which equals an IOR of 1.5
  
    mat.diffuse = lerp(mat.albedo.rgb * (float3(1.0,1.0,1.0) - dielectricSpecular), black, mat.metallic);
    mat.F0 = lerp(dielectricSpecular, mat.albedo.rgb, mat.metallic);
    
    // perceptual roughness to roughness
    mat.a = mat.roughness * mat.roughness;

    // prevent division by 0
    mat.a = max(mat.a, 0.01);
    return mat;
}

Material getMaterial(float2 texcoord)
{
    Material mat;

    // Read textures/uniforms

    mat.albedo = materialAlbedoValue(texcoord);
    float2 metallicRoughness = materialMetallicRoughnessValue(texcoord);
    mat.metallic  = metallicRoughness.r;
    mat.roughness = metallicRoughness.g;
    mat.normal = materialNormalValue(texcoord);
    mat.occlusion = materialOcclusionValue(texcoord);
    mat.emissive = materialEmissiveValue(texcoord);

    mat = initMaterial(mat);

    return mat;
}

// Reduce specular aliasing by producing a modified roughness value
// Tokuyoshi et al. 2019. Improved Geometric Specular Antialiasing.
// http://www.jp.square-enix.com/tech/library/pdf/ImprovedGeometricSpecularAA.pdf
float specularAntiAliasing(float3 N, float a)
{
    // normal-based isotropic filtering
    // this is originally meant for deferred rendering but is a bit simpler to implement than the forward version
    // saves us from calculating uv offsets and sampling textures for every light

    const float SIGMA2 = 0.25; // squared std dev of pixel filter kernel (in pixels)
    const float KAPPA  = 0.18; // clamping threshold
    float3 dndu = ddx(N);
    float3 dndv = ddy(N);
    float variance = SIGMA2 * (dot(dndu, dndu) + dot(dndv, dndv));
    float kernelRoughness2 = min(2.0 * variance, KAPPA);
    return saturate(a + kernelRoughness2);
}

// Physically based shading
// Metallic + roughness workflow (GLTF 2.0 core material spec)
// BRDF, no sub-surface scattering
// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#metallic-roughness-material
// Some GLSL code taken from
// https://google.github.io/filament/Filament.md.html
// and
// https://learnopengl.com/PBR/Lighting

// Schlick approximation to Fresnel equation
float3 F_Schlick(float VoH, float3 F0)
{
    float f = pow(1.0 - VoH, 5.0);
    return f + F0 * (1.0 - f);
}

// Normal Distribution Function
// (aka specular distribution)
// distribution of microfacets

// Bruce Walter et al. 2007. Microfacet Models for Refraction through Rough Surfaces.
// equivalent to Trowbridge-Reitz
float D_GGX(float NoH, float a)
{
    a = NoH * a;
    float k = a / (1.0 - NoH * NoH + a * a);
    return k * k * INV_PI;
}

// Visibility function
// = Geometric Shadowing/Masking Function G, divided by the denominator of the Cook-Torrance BRDF (4 NoV NoL)
// G is the probability of the microfacet being visible in the outgoing (masking) or incoming (shadowing) direction

// Heitz 2014. Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs.
// http://jcgt.org/published/0003/02/03/paper.pdf
// based on height-correlated Smith-GGX
float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
    float a2 = a * a;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}

// version without height-correlation
float V_SmithGGX(float NoV, float NoL, float a)
{
    float a2 = a * a;
    float GGXV = NoV + sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoL + sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 1.0 / (GGXV * GGXL);
}


// Lambertian diffuse BRDF
// uniform color
float Fd_Lambert()
{
    // normalize to conserve energy
    // cos integrates to pi over the hemisphere
    // incoming light is multiplied by cos and BRDF
    return INV_PI;
}

// Account for multiple scattering across microfacets
// Computes a scaling factor for the BRDF

// Turquin. 2018. Practical multiple scattering compensation for microfacet models.
// https://blog.selfshadow.com/publications/turquin/ms_comp_final.pdf
float3 multipleScatteringFactor(Material mat, float NoV)
{
    if(u_multipleScattering)
    {
        // Turquin approximates the multiple scattering portion of the BRDF using a scaled down version of the single scattering BRDF
        // That scale factor is E: the directional albedo for single scattering, ie. the total reflectance for a viewing direction
        float2 E = s_texAlbedoLUT.Sample(s_texAlbedoLUTState, float2(NoV, mat.a)).xy;

        // for metals, the albedo value is calculated with F = 1 (perfect reflection)
        // fresnel determines whether light is reflected or absorbed
        float3 factorMetallic = float3(1.0,1.0,1.0) + mat.F0 * (1.0 / E.x - 1.0);

        // for dielectrics, fresnel determines the ratio between specular and diffuse energy
        // so the albedo depends on F as a variable
        // however, dielectrics in GLTF have a fixed F0 of 0.04 so we can do this with a second LUT
        float3 factorDielectric = float3(1.0 / E.y, 1.0 / E.y, 1.0 / E.y);

        return lerp(factorDielectric, factorMetallic, mat.metallic);
    }
    else return float3(1.0,1.0,1.0);
}

bool whiteFurnaceEnabled()
{
    return u_whiteFurnace;
}

// White furnace test: lighting integral against a constant white environment
// Used to visualize energy loss/gain
// This is exactly what the multiple scattering LUT calculates
float3 whiteFurnace(float NoV, Material mat)
{
    float2 Es = s_texAlbedoLUT.Sample(s_texAlbedoLUTState, float2(NoV, mat.a)).xy;
    float E = lerp(Es.y, Es.x, mat.metallic);
    return E * float3(u_whiteFurnaceRadiance, u_whiteFurnaceRadiance, u_whiteFurnaceRadiance);
}

// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#appendix-b-brdf-implementation
float3 BRDF(float3 v, float3 l, float3 n, float NoV, float NoL, Material mat)
{
    // V is the normalized vector from the shading location to the eye
    // L is the normalized vector from the shading location to the light
    // N is the surface normal in the same space as the above values
    // H is the half vector, where H = normalize(L+V)

    float3 h = normalize(l + v);
    float NoH = saturate(dot(n, h));
    float VoH = saturate(dot(v, h));

    // specular BRDF
    float D = D_GGX(NoH, mat.a);
    float3 F = F_Schlick(VoH, mat.F0);
    float V = V_SmithGGXCorrelated(NoV, NoL, mat.a);
    float3 Fr = F * (V * D);

    // diffuse BRDF
    float3 Fd = mat.diffuse * Fd_Lambert();

    return Fr + (1.0 - F) * Fd;
}
