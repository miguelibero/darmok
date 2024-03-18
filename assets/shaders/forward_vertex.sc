$input a_position, a_normal, a_tangent, a_color0, a_texcoord0
$output v_position, v_view, v_normal, v_tangent, v_color0, v_texcoord0

#include <bgfx_shader.sh>

void main()
{
	vec4 pos = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_position = pos.xyz;
	v_view = mul(u_modelView, vec4(a_position, 1.0) ).xyz;
	v_normal = mul(u_modelView, vec4(a_normal, 0.0) ).xyz;
	v_tangent = mul(u_modelView, vec4(a_tangent, 0.0)).xyz;
	v_texcoord0 = a_texcoord0;
	v_color0 = a_color0;
	gl_Position = pos;
}