$input a_position, a_normal, a_color0, a_texcoord0, a_indices, a_weight
$output v_position, v_normal, v_color0, v_texcoord0

#include <bgfx_shader.sh>

#if SKINNING_ENABLED
#include <darmok_skinning.include.sc>
#endif

void main()
{
	vec4 pos = vec4(a_position, 1.0);
	vec4 norm = vec4(a_normal, 0.0);
#if SKINNING_ENABLED	
	pos = applySkinning(pos, a_indices, a_weight);
	norm = applySkinning(norm, a_indices, a_weight);
#endif
	pos = mul(u_model[0], pos);
	norm = mul(u_model[0], norm);
	v_position = pos.xyz;
	v_normal = norm.xyz;
	v_texcoord0 = a_texcoord0;
	v_color0 = a_color0;
	gl_Position = mul(u_viewProj, pos);
}