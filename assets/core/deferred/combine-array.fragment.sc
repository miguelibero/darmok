$input v_texcoord0

#include "common.sh"

SAMPLER2DARRAY(s_albedo, 0);
SAMPLER2D(s_light,  1);

void main()
{
	vec4 albedo  = toLinear(texture2DArray(s_albedo, vec3(v_texcoord0, 0.0) ) );
	vec4 light   = toLinear(texture2D(s_light,  v_texcoord0) );
	gl_FragColor = toGamma(albedo*light);
}
