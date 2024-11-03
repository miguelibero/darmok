$input v_position, v_normal, v_texcoord0, v_viewDir

#include <bgfx_shader.sh>
#include <darmok_material_basic.sc>
#include <darmok_light.sc>

#if DARMOK_VARIANT_SHADOW_ENABLED
#include <darmok_shadow.sc>
#endif

vec3 calcRadiance(vec3 radianceIn, vec3 V, vec3 L, vec3 N, Material mat)
{
	vec3 diffuse = phongDiffuse(L, N, radianceIn);
	vec3 specular = phongSpecular(V, L, N, radianceIn, mat.shininess);

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
		float dist = distance(light.position, fragPos);
        float attenuation = smoothAttenuation(dist, light.range);
        if(attenuation > 0.0)
        {
			vec3 L = normalize(light.position - fragPos);
			vec3 radianceIn = light.intensity * attenuation;
			radianceOut += calcRadiance(radianceIn, V, L, N, mat);
		}
	}

	uint spotLights = spotLightCount();
    for(uint i = 0; i < spotLights; i++)
    {
        SpotLight light = getSpotLight(i);
        vec3 L = light.position - fragPos;
        float dist = length(L);
        L = L / dist;
        float cosTheta = dot(L, -light.direction);
        float innerCutoff = cos(light.innerConeAngle);
        float outerCutoff = cos(light.coneAngle);

        float attenuation = 0.0;
        if (cosTheta > outerCutoff)
        {
            float factor = (cosTheta - outerCutoff) / (innerCutoff - outerCutoff);
            attenuation = factor * smoothAttenuation(dist, light.range);
        }

        if(attenuation > 0.0)
        {
            vec3 radianceIn = light.intensity * attenuation;
            radianceOut += calcRadiance(radianceIn, V, L, N, mat);
        }
    }


	uint dirLights = dirLightCount();
    for(uint i = 0; i < dirLights; i++)
    {
		DirectionalLight light = getDirLight(i);
		vec3 L = -light.direction;
		vec3 radianceIn = light.intensity;

		radianceOut += calcRadiance(radianceIn, V, L, N, mat);

#if DARMOK_VARIANT_SHADOW_ENABLED
		float shadowBias = normalShadowBias(N, L);  
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