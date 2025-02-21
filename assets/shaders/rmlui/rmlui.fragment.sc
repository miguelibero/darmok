$input v_texcoord0, v_color0

#include <bgfx_shader.sh>

#ifndef DARMOK_VARIANT_TEXTURE_DISABLE
SAMPLER2D(s_texColor,  0);
#endif

void main()
{
	gl_FragColor = v_color0;
#ifndef DARMOK_VARIANT_TEXTURE_DISABLE
	gl_FragColor *= texture2D(s_texColor, v_texcoord0);
#endif
}