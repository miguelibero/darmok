$input v_texcoord0, v_color0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

uniform vec4 u_color;

void main()
{
    gl_FragColor = v_color0 * u_color * texture2D(s_texColor, v_texcoord0);
}