#ifndef DARMOK_FORWARD_BASIC_HEADER
#define DARMOK_FORWARD_BASIC_HEADER

#include <darmok_material_basic.sc>
#include <darmok_light.sc>

#if DARMOK_VARIANT_SHADOW_ENABLED
#include <darmok_shadow.sc>
#endif

vec3 calcRadiance(LightData data, vec3 V, vec3 N, Material mat)
{
	vec3 diffuse = phongDiffuse(data.direction, N, data.radiance);
	vec3 specular = phongSpecular(V, data.direction, N, data.radiance, mat.shininess);
	return (diffuse * mat.diffuse) + (specular * mat.specular);
}

vec4 forwardFragment(vec3 V, vec3 N, vec3 fragPos, vec2 texCoord)
{
	V = normalize(V);
	N = normalize(N);
    Material mat = getMaterial(texCoord);
    vec3 radianceOut = vec3_splat(0);

#if DARMOK_VARIANT_SHADOW_ENABLED
	float visibility = 0;
#endif

	uint pointLights = pointLightCount();
    for(uint i = 0; i < pointLights; i++)
    {
		PointLight light = getPointLight(i);
		LightData data = calcPointLight(light, fragPos);
		radianceOut += calcRadiance(data, V, N, mat);
	}

	uint spotLights = spotLightCount();
    for(uint i = 0; i < spotLights; i++)
    {
        SpotLight light = getSpotLight(i);
        LightData data = calcSpotLight(light, fragPos);
		radianceOut += calcRadiance(data, V, N, mat);
    }

	uint dirLights = dirLightCount();
    for(uint i = 0; i < dirLights; i++)
    {
		DirLight light = getDirLight(i);
        LightData data = calcDirLight(light);
		radianceOut += calcRadiance(data, V, N, mat);

#if DARMOK_VARIANT_SHADOW_ENABLED
		float shadowBias = normalShadowBias(N, data.direction);  
		visibility += softShadow(i, fragPos, shadowBias);
		// visibility += hardShadow(i, fragPos, shadowBias);
#endif
	}

#if DARMOK_VARIANT_SHADOW_ENABLED
    if(dirLights > 0)
    {
		radianceOut.rgb *= visibility / dirLights;
	}
#endif

    radianceOut += getAmbientLight().irradiance * mat.diffuse;
    return vec4(radianceOut, mat.diffuse.a);
}

#endif // DARMOK_FORWARD_BASIC_HEADER


