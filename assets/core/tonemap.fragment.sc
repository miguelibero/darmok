$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture2D(s_texColor, v_texcoord0).rgb;
  
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3_splat(1.0));
    
    // gamma correction 
    mapped = pow(mapped, vec3_splat(1.0 / gamma));
  
    gl_FragColor = vec4(mapped, 1.0);
}