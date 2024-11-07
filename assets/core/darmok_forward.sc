#ifndef DARMOK_FORWARD_HEADER
#define DARMOK_FORWARD_HEADER

#define READ_MATERIAL

#include <darmok_material.sc>
#include <darmok_light.sc>
#include <darmok_shadow.sc>

vec3 calcLightRadiance(LightData data, vec3 V, vec3 N, float NoV, vec3 msFactor, Material mat)
{
    float NoL = saturate(dot(N, data.direction));
    return BRDF(V, data.direction, N, NoV, NoL, mat) * msFactor * data.radiance * NoL;
}

vec4 forwardFragment(vec3 V, vec3 normal, vec3 tangent, vec3 fragPos, vec2 texCoord)
{
    V = normalize(V);
    Material mat = getMaterial(texCoord);

    // convert normal map from tangent space -> world space (= space of tangent, etc.)
    vec3 N = convertTangentNormal(normal, tangent, mat.normal);
    mat.a = specularAntiAliasing(N, mat.a);

    float NoV = abs(dot(N, V)) + 1e-5;

    vec3 msFactor = multipleScatteringFactor(mat, NoV);
    vec3 radianceOut = vec3_splat(0.0);

    if(whiteFurnaceEnabled())
    {
        mat.F0 = vec3_splat(1.0);
        radianceOut += whiteFurnace(NoV, mat) * msFactor;
        return vec4(radianceOut, 1.0);
    }

    uint pointLights = pointLightCount();
    for(uint i = 0; i < pointLights; i++)
    {
        float visibility = pointLightShadowVisibility(i, fragPos);
        if(visibility <= 0.0)
        {
            continue;
        }
        PointLight light = getPointLight(i);
        LightData data = calcPointLight(light, fragPos);
        radianceOut += calcLightRadiance(data, V, N, NoV, msFactor, mat) * visibility;
    }

    uint spotLights = spotLightCount();
    for(uint i = 0; i < spotLights; i++)
    {
        float visibility = spotLightShadowVisibility(i, fragPos);
        if(visibility <= 0.0)
        {
            continue;
        }
        SpotLight light = getSpotLight(i);
        LightData data = calcSpotLight(light, fragPos);
        radianceOut += calcLightRadiance(data, V, N, NoV, msFactor, mat) * visibility;
    }

    uint dirLights = dirLightCount();
    for(uint i = 0; i < dirLights; i++)
    {
        float visibility = dirLightShadowVisibility(i, fragPos, N);
        if(visibility <= 0.0)
        {
            continue;
        }
        DirLight light = getDirLight(i);
        LightData data = calcDirLight(light);
        radianceOut += calcLightRadiance(data, V, N, NoV, msFactor, mat) * visibility;
    }

    radianceOut += getAmbientLight().irradiance * mat.diffuse * mat.occlusion;
    radianceOut += mat.emissive;

    return vec4(radianceOut, mat.albedo.a);
}

#endif // DARMOK_FORWARD_HEADER


