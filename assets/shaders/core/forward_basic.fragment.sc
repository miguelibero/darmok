$input v_position, v_normal, v_texcoord0, v_viewDir

#include <darmok_forward_basic.sc>

void main()
{
    gl_FragColor = forwardFragment(v_viewDir, v_normal, v_position, v_texcoord0);
}