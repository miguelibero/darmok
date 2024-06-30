$input v_color0

#include <bgfx_shader.sh>

uniform vec4 u_diffuseColor;

void main()
{
	gl_FragColor = v_color0 * u_diffuseColor;
}