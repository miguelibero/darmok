#if TEXTURE_ENABLED
$input v_color0, v_texcoord0
#else
$input v_color0
#endif

#include <bgfx_shader.sh>

#if TEXTURE_ENABLED
SAMPLER2D(s_texColor, 0);
#endif

uniform vec4 u_diffuseColor;

void main()
{
	gl_FragColor = v_color0 * u_diffuseColor;
#if TEXTURE_ENABLED
	gl_FragColor *= texture2D(s_texColor, v_texcoord0);
#endif
}