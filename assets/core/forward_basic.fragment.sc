$input v_position, v_normal, v_texcoord0, v_viewDir

#include <bgfx_shader.sh>
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

void main()
{
    Material mat = getMaterial(v_texcoord0);

    vec3 V = v_viewDir;
	vec3 N = v_normal;
    vec3 radianceOut = vec3_splat(0);
	vec3 fragPos = v_position;

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
		gl_FragColor.rgb *= visibility / dirLights;
	}
#endif

    radianceOut += getAmbientLight().irradiance * mat.diffuse;
    gl_FragColor.rgb = radianceOut;
    gl_FragColor.a = mat.diffuse.a;
}