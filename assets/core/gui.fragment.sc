$input v_texcoord0, v_color0

#include <darmok.include.sc>

#if TEXTURE_ENABLED
SAMPLER2D(s_texColor, 0);
#endif

void main()
{
    gl_FragColor = v_color0;
#if TEXTURE_ENABLED
    gl_FragColor * texture2D(s_texColor, v_texcoord0);
#endif
}