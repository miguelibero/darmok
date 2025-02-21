$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main()
{
    vec4 baseColor = texture2D(s_texColor, v_texcoord0);
    vec3 hdrColor = baseColor.rgb;
  
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3_splat(1.0));
    
    // gamma correction
    const float gamma = 2.2;
    mapped = pow(mapped, vec3_splat(1.0 / gamma));
  
    gl_FragColor = vec4(mapped, baseColor.a);
}