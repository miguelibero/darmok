$input v_position, v_normal, v_texcoord0, v_viewDir

#include <bgfx_shader.sh>
#include <darmok_material_basic.sc>
#include <darmok_light.sc>

#if DARMOK_VARIANT_SHADOW_ENABLED
#include <darmok_shadow.sc>
#endif

void main()
{
    Material mat = getMaterial(v_texcoord0);

    vec3 diffuse = vec3_splat(0);
	vec3 specular = vec3_splat(0);
	vec3 ambient = getAmbientLight().irradiance;
	vec3 norm = v_normal;
	vec3 fragPos = v_position;
	vec3 viewDir = v_viewDir;

#if DARMOK_VARIANT_SHADOW_ENABLED
	float visibility = 0;
#endif

	uint pointLights = pointLightCount();
    for(uint i = 0; i < pointLights; i++)
    {
		PointLight light = getPointLight(i);
		float dist = distance(light.position, fragPos);
        float attenuation = smoothAttenuation(dist, light.radius);
        if(attenuation > 0.0)
        {
			vec3 lightDir = normalize(light.position - fragPos);
			vec3 radianceIn = light.intensity * attenuation;
			diffuse += phongDiffuse(lightDir, norm, radianceIn);
			specular += phongSpecular(lightDir, norm, viewDir, radianceIn, mat.shininess);
		}
	}

	uint dirLights = dirLightCount();
    for(uint i = 0; i < dirLights; i++)
    {
		DirectionalLight light = getDirLight(i);
		vec3 lightDir = -light.direction;
		vec3 radianceIn = light.intensity;

		diffuse += phongDiffuse(lightDir, norm, radianceIn);
		specular += phongSpecular(lightDir, norm, viewDir, radianceIn, mat.shininess);

#if DARMOK_VARIANT_SHADOW_ENABLED
		float shadowBias = normalShadowBias(norm, lightDir);  
		visibility += softShadow(i, fragPos, shadowBias);
		// visibility += hardShadow(i, fragPos, shadowBias);
#endif
	}

	ambient *= mat.diffuse;
	diffuse *= mat.diffuse;
	specular *= mat.specular;

	gl_FragColor.rgb = diffuse + specular;

#if DARMOK_VARIANT_SHADOW_ENABLED
	gl_FragColor.rgb *= visibility / float(dirLights);
#endif

	gl_FragColor.rgb += ambient;
	gl_FragColor.a = mat.diffuse.a;
}