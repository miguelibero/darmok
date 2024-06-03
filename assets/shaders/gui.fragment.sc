$input v_texcoord0, v_color0

#include <darmok.include.sc>

SAMPLER2D(s_texColor, 0);

void main()
{
    gl_FragColor = v_color0 * texture2D(s_texColor, v_texcoord0);
}