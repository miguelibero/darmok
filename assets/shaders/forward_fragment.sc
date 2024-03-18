$input v_color0, v_texcoord0, v_position, v_view, v_normal 

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);
uniform vec4 u_diffuseColor;

void main()
{
	vec4 texel = texture2D(s_texColor, v_texcoord0);
	gl_FragColor = texel * v_color0 * u_diffuseColor;
}