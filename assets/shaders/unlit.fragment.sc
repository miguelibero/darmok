$input v_color0, v_texcoord0

#include <darmok.include.sc>

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