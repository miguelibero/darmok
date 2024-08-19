$input v_position, v_normal, v_texcoord0, v_viewDir

#include <bgfx_shader.sh>
#include <darmok_material_basic.sc>
#include <darmok_light.sc>

void main()
{
    Material mat = getMaterial(v_texcoord0);

    vec3 diffuse = vec3_splat(0);
	vec3 specular = vec3_splat(0);
	vec3 ambient = getAmbientLight().irradiance;
	vec3 norm = normalize(v_normal);
	vec3 fragPos = v_position;
	vec3 viewDir = v_viewDir;

	uint lights = pointLightCount();
    for(uint i = 0; i < lights; i++)
    {
		PointLight light = getPointLight(i);
		float dist = distance(light.position, fragPos);

		vec3 lightDir = normalize(light.position - fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		diffuse += diff * light.intensity;

		vec3 reflectDir = reflect(-lightDir, norm);  
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
		specular += spec * light.intensity;
	}
	
	ambient *= mat.diffuse;
	diffuse *= mat.diffuse;
	specular *= mat.specular;

	gl_FragColor.rgb = ambient + diffuse + specular;
	gl_FragColor.a = mat.diffuse.a;
}
