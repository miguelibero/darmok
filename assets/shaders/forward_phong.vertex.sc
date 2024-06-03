$input a_position, a_normal, a_color0, a_texcoord0, a_indices, a_weight
$output v_position, v_normal, v_color0, v_texcoord0

#include <darmok.include.sc>
#include <darmok_skinning.include.sc>

// https://learnopengl.com/Lighting/Basic-Lighting

void main()
{
	vec4 pos = vec4(a_position, 1.0);
	vec4 norm = vec4(a_normal, 0.0);
	pos = applySkinning(pos, a_indices, a_weight);
	norm = applySkinning(norm, a_indices, a_weight);
	pos = mul(u_model[0], pos);
	norm = mul(u_model[0], norm);
	v_position = pos.xyz;
	v_normal = norm.xyz;
	v_texcoord0 = a_texcoord0;
	v_color0 = a_color0;
	gl_Position = mul(u_viewProj, pos);
}