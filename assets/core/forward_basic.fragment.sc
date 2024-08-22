$input v_position, v_normal, v_texcoord0, v_viewDir

#include <bgfx_shader.sh>
#include <darmok_material_basic.sc>
#include <darmok_light.sc>
#include <darmok_shadow.sc>

void main()
{
    Material mat = getMaterial(v_texcoord0);

    vec3 diffuse = vec3_splat(0);
	vec3 specular = vec3_splat(0);
	vec3 ambient = getAmbientLight().irradiance;
	vec3 norm = normalize(v_normal);
	vec3 fragPos = v_position;
	vec3 viewDir = v_viewDir;

	vec2 shadowMapTexelSize = u_shadowMapData.xy;
	float visibility = 0;

	uint pointLights = pointLightCount();
    for(uint i = 0; i < pointLights; i++)
    {
		PointLight light = getPointLight(i);
		vec3 lightDir = normalize(light.position - fragPos);

		diffuse += phongDiffuse(lightDir, norm, light.intensity);
		specular += phongSpecular(lightDir, norm, viewDir, light.intensity, mat.shininess);
	}

	uint dirLights = dirLightCount();
    for(uint i = 0; i < dirLights; i++)
    {
		DirectionalLight light = getDirLight(i);
		vec3 lightDir = -light.direction;


		diffuse += phongDiffuse(lightDir, norm, light.intensity);
		specular += phongSpecular(lightDir, norm, viewDir, light.intensity, mat.shininess);

		float shadowBias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);  
		vec4 shadowCoord = mul(u_dirLightTrans, vec4(fragPos, 1.0));
		// visibility += hardShadow(s_shadowMap, shadowCoord, shadowBias);
		visibility += PCF(s_shadowMap, shadowCoord, shadowBias, shadowMapTexelSize);
	}

	ambient *= mat.diffuse;
	diffuse *= mat.diffuse;
	specular *= mat.specular;

	visibility /= float(dirLights);

	gl_FragColor.rgb = ambient + visibility * (diffuse + specular);
	gl_FragColor.a = mat.diffuse.a;
}