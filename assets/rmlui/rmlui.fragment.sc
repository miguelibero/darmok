#if TEXTURE_ENABLE
$input v_texcoord0, v_color0
#else
$input v_color0
#endif

#include <darmok.include.sc>

#if TEXTURE_ENABLE
SAMPLER2D(s_texColor,  0);
#endif

void main()
{
#if TEXTURE_ENABLE
	vec4 tex = texture2D(s_texColor, v_texcoord0);
	gl_FragColor = v_color0 * tex;
#else
	gl_FragColor = v_color0;
#endif
}