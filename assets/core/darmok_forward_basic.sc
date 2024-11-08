#ifndef DARMOK_FORWARD_BASIC_HEADER
#define DARMOK_FORWARD_BASIC_HEADER

#include <darmok_material_basic.sc>
#include <darmok_light.sc>
#include <darmok_shadow.sc>

vec3 calcLightRadiance(LightData data, vec3 V, vec3 N, Material mat)
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

	uint pointLights = pointLightCount();
    for(uint i = 0; i < pointLights; i++)
    {
		PointLight light = getPointLight(i);
		LightData data = calcPointLight(light, fragPos);
		float visibility = pointLightShadow(light.entity, fragPos, N, data.direction);
        if(visibility <= 0.0)
        {
            continue;
        }
		radianceOut += calcLightRadiance(data, V, N, mat) * visibility;
	}

	uint spotLights = spotLightCount();
    for(uint i = 0; i < spotLights; i++)
    {
        SpotLight light = getSpotLight(i);
        LightData data = calcSpotLight(light, fragPos);
		float visibility = spotLightShadow(light.entity, fragPos, N, data.direction);
        if(visibility <= 0.0)
        {
            continue;
        }
		radianceOut += calcLightRadiance(data, V, N, mat) * visibility;
    }

	uint dirLights = dirLightCount();
    for(uint i = 0; i < dirLights; i++)
    {
		DirLight light = getDirLight(i);
        LightData data = calcDirLight(light);
		float visibility = dirLightShadow(light.entity, fragPos, N, data.direction);
        if(visibility <= 0.0)
        {
            continue;
        }
		radianceOut += calcLightRadiance(data, V, N, mat) * visibility;
	}

    radianceOut += getAmbientLight().irradiance * mat.diffuse;
    return vec4(radianceOut, mat.diffuse.a);
}

#endif // DARMOK_FORWARD_BASIC_HEADER


