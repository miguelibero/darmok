$input a_position, a_color0, a_texcoord0, a_indices, a_weight
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <darmok_skinning.sc>

void main()
{
	vec4 pos = vec4(a_position, 1.0);
	pos = applySkinning(pos, a_indices, a_weight);
	gl_Position = mul(u_modelViewProj, pos);
	v_color0 = a_color0;
	v_texcoord0 = a_texcoord0;
}