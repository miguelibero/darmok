$input v_position, v_normal, v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include "phong_lighting.include.sc"

SAMPLER2D(s_texColor, 0);
uniform vec4 u_diffuseColor;
uniform vec4 u_specularColor;
uniform vec4 u_camPos;

// https://learnopengl.com/Lighting/Basic-Lighting

void main()
{
	vec4 base = texture2D(s_texColor, v_texcoord0) * v_color0;
	vec3 diffuse = vec3_splat(0);
	vec3 specular = vec3_splat(0);
	vec3 viewDir = normalize(u_camPos.xyz - v_position);
	vec3 norm = normalize(v_normal);
	Material material = getMaterial();

	uint c = getPointLightCount();
    for(uint i = 0; i < c; i++)
    {
		PointLight light = getPointLight(i);

		vec3 lightDir = normalize(light.position - v_position);
		float diff = max(dot(norm, lightDir), 0.0);
		diffuse += diff * light.diffuse;

		vec3 reflectDir = reflect(-lightDir, norm);  
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		specular += material.specularStrength * spec * light.specular;
	}
	
	diffuse *= u_diffuseColor.xyz;
	specular *= u_specularColor.xyz;
	vec3 ambient = getAmbientLight().color;

	gl_FragColor.rgb = base.rgb * (ambient + diffuse + specular);
	gl_FragColor.a = base.a;
}