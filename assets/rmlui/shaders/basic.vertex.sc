$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_color0

#include <bgfx_shader.sh>

void main()
{
	vec4 pos = mul(u_modelViewProj, vec4(a_position.xy, 0.0, 1.0));
	// gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
	gl_Position = pos;
	v_texcoord0 = a_texcoord0;
	v_color0    = a_color0;
}