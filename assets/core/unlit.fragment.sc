$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

uniform vec4 u_color;

void main()
{
	gl_FragColor = v_color0 * u_color;
#ifndef DARMOK_VARIANT_TEXTURE_DISABLED
	gl_FragColor *= texture2D(s_texColor, v_texcoord0);
#endif
}