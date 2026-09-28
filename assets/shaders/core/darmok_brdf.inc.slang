#pragma once

#define PI     (3.14159265359)
#define INV_PI (0.31830988618)

struct Material
{
    float4 albedo;
    float  metallic;
    float  roughness;
    float3 normal;
    float  occlusion;
    float3 emissive;

    // calculated from the above
    float3 diffuse; // becomes black for higher metalness
    float3 F0;      // Fresnel reflectance at normal incidence
    float  a;       // remapped roughness (^2)
};

Material initMaterial(Material mat)
{
    const float3 dielectricSpecular = float3(0.04, 0.04, 0.04);
    const float3 black = float3(0.0, 0.0, 0.0);
    mat.diffuse = lerp(mat.albedo.rgb * (float3(1.0, 1.0, 1.0) - dielectricSpecular), black, mat.metallic);
    mat.F0      = lerp(dielectricSpecular, mat.albedo.rgb, mat.metallic);
    mat.a       = max(mat.roughness * mat.roughness, 0.01);
    return mat;
}

// Tokuyoshi et al. 2019. Improved Geometric Specular Antialiasing.
float specularAntiAliasing(float3 N, float a)
{
    const float SIGMA2 = 0.25;
    const float KAPPA  = 0.18;
    float3 dndu = ddx(N);
    float3 dndv = ddy(N);
    float variance = SIGMA2 * (dot(dndu, dndu) + dot(dndv, dndv));
    float kernelRoughness2 = min(2.0 * variance, KAPPA);
    return saturate(a + kernelRoughness2);
}

float3 F_Schlick(float VoH, float3 F0)
{
    float f = pow(1.0 - VoH, 5.0);
    return f + F0 * (1.0 - f);
}

float D_GGX(float NoH, float a)
{
    float a2 = a * a;
    float f  = NoH * NoH * (a2 - 1.0) + 1.0;
    return a2 / (PI * f * f);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
    float a2   = a * a;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}

float V_SmithGGX(float NoV, float NoL, float a)
{
    float a2   = a * a;
    float GGXV = NoV + sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoL + sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 1.0 / (GGXV * GGXL);
}

float Fd_Lambert()
{
    return INV_PI;
}

float3 BRDF(float3 v, float3 l, float3 n, float NoV, float NoL, Material mat)
{
    float3 h   = normalize(l + v);
    float  NoH = saturate(dot(n, h));
    float  VoH = saturate(dot(v, h));

    float  D  = D_GGX(NoH, mat.a);
    float3 F  = F_Schlick(VoH, mat.F0);
    float  V  = V_SmithGGXCorrelated(NoV, NoL, mat.a);
    float3 Fr = F * (V * D);
    float3 Fd = mat.diffuse * Fd_Lambert();

    return Fr + (1.0 - F) * Fd;
}
