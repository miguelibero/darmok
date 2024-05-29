$input a_position, a_normal, a_color0, a_texcoord0, a_indices, a_weight
$output v_position, v_normal, v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include "skeleton.include.sc"

// https://learnopengl.com/Lighting/Basic-Lighting

void main()
{
	vec4 pos = vec4(a_position, 1.0);
	pos = mul(getSkinningMatrix(a_indices, a_weight), pos);

	v_position = mul(u_model[0], pos).xyz;
	v_normal = mul(u_model[0], vec4(a_normal, 0.0)).xyz;
	v_texcoord0 = a_texcoord0;
	v_color0 = a_color0;
	gl_Position = mul(u_modelViewProj, pos);
}