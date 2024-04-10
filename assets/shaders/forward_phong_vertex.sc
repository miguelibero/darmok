$input a_position, a_normal, a_color0, a_texcoord0
$output v_position, v_view, v_normal, v_color0, v_texcoord0

#include <bgfx_shader.sh>

// https://learnopengl.com/Lighting/Basic-Lighting

void main()
{
	vec4 pos = mul(u_modelViewProj, vec4(a_position, 1.0));
	vec3 normal = a_normal * 2.0 - 1.0;
	mat4 model = u_model[0];
	v_position = pos.xyz;
	v_view = mul(u_modelView, vec4(a_position, 1.0) ).xyz;
	v_normal = mul(model, vec4(normal, 0.0) ).xyz;
	v_texcoord0 = a_texcoord0;
	v_color0 = a_color0;
	gl_Position = pos;
}