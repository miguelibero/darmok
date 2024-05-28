$input a_position, a_normal, a_color0, a_texcoord0, a_indices, a_weight
$output v_position, v_normal, v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include "skeleton.include.sc"

// https://learnopengl.com/Lighting/Basic-Lighting

void main()
{
	vec4 localPos = vec4(a_position, 1.0);
	localPos = mul(getSkinningMatrix(a_indices, a_weight), localPos);
	
	vec4 pos = mul(u_modelViewProj, localPos);
	v_position = mul(u_model[0], localPos).xyz;
	v_normal = mul(u_model[0], vec4(a_normal, 0.0)).xyz;
	v_texcoord0 = a_texcoord0;
	v_color0 = a_color0;
	gl_Position = pos;
}