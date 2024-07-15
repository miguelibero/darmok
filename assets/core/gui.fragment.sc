$input v_texcoord0, v_color0

#include <bgfx_shader.sh>

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