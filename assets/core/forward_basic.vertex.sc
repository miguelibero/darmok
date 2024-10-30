$input a_position, a_normal, a_texcoord0, a_indices, a_weight
$output v_position, v_normal, v_texcoord0, v_viewDir

#include <bgfx_shader.sh>

#if DARMOK_VARIANT_SKINNING_ENABLED
#include <darmok_skinning.sc>
#endif

uniform vec4 u_camPos;
uniform mat3 u_normalMatrix;

void main()
{
	vec4 pos = vec4(a_position, 1.0);
	vec4 norm = vec4(a_normal, 0.0);
#if DARMOK_VARIANT_SKINNING_ENABLED
	pos = applySkinning(pos, a_indices, a_weight);
	norm = applySkinning(norm, a_indices, a_weight);
#endif

	pos = mul(u_model[0], pos);
	v_position = pos.xyz;
	v_normal = normalize(mul(u_normalMatrix, norm.xyz));
	v_viewDir = normalize(u_camPos.xyz - v_position);
	v_texcoord0 = a_texcoord0;
	gl_Position = mul(u_viewProj, pos);
}