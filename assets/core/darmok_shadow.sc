
#include <bgfx_shader.sh>

SAMPLER2DSHADOW(s_shadowMap, DARMOK_SAMPLER_SHADOW_MAP);

float hardShadow(mat4 lightTrans, vec3 pos)
{
    vec4 pos4 = mul(lightTrans, vec4(pos, 1.0));
    return shadow2DProj(s_shadowMap, pos4).r;
}