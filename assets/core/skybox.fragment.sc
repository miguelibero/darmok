$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLERCUBE(s_texColor, 0);

void main()
{
    gl_FragColor = textureCube(s_texColor, v_texcoord0);
}