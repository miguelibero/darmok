$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main()
{
	vec4 texel = texture2D(s_texColor, v_texcoord0);
	gl_FragColor = texel * v_color0;
}