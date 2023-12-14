$input a_position

#include <bgfx_shader.sh>

void main()
{
	vec4 pos = mul(u_modelViewProj, vec4(a_position, 0.0, 1.0) );
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}