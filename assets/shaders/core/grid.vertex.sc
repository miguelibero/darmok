$input a_position
$output v_nearPoint, v_farPoint, v_texcoord0

#include <bgfx_shader.sh>

vec3 unproject(vec3 pos)
{
    vec4 p = mul(u_invViewProj, vec4(pos, 1.0));
    return p.xyz / p.w;
}

void main()
{
	vec4 pos = vec4(a_position, 0.0, 1.0);
	v_nearPoint = unproject(vec3(a_position, 0.0));
    v_farPoint = unproject(vec3(a_position, 1.0));
	v_texcoord0 = a_position;
	gl_Position = pos;
}