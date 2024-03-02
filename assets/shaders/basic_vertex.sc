$input a_color0, a_texcoord0, a_position, a_normal
$output v_color0, v_texcoord0, v_position, v_normal, v_view

#include <bgfx_shader.sh>

void main()
{
	vec3 normal = a_normal.xyz * 2.0 - 1.0;
	vec4 pos = mul(u_modelViewProj, vec4(a_position, 1.0));
	gl_Position = pos;
	v_position = pos.xyz;
	v_view = mul(u_modelView, vec4(a_position, 1.0) ).xyz;
	v_normal = mul(u_modelView, vec4(normal, 0.0) ).xyz;
	v_texcoord0 = a_texcoord0;
	v_color0 = a_color0;
}