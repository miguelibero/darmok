$input a_position, a_color0, a_texcoord0, a_indices, a_weight
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>

#if SKINNING_ENABLED
#include <darmok_skinning.include.sc>
#endif

void main()
{
	vec4 pos = vec4(a_position, 1.0);
#if SKINNING_ENABLED	
	pos = applySkinning(pos, a_indices, a_weight);
#endif
	gl_Position = mul(u_modelViewProj, pos);
	v_color0    = a_color0;
	v_texcoord0 = a_texcoord0;
}