$input a_position, a_normal, a_tangent, a_texcoord0
$output v_wpos, v_view, v_normal, v_tangent, v_bitangent, v_texcoord0

#include <bgfx_shader.sh>

void main()
{
	vec3 wpos = mul(u_model[0], vec4(a_position, 1.0) ).xyz;
	gl_Position = mul(u_viewProj, vec4(wpos, 1.0) );
	
	vec3 wnormal = mul(u_model[0], vec4(a_normal, 0.0) ).xyz;
	vec3 wtangent = mul(u_model[0], vec4(a_tangent, 0.0) ).xyz;

	vec3 viewNormal = normalize(mul(u_view, vec4(wnormal, 0.0) ).xyz);
	vec3 viewTangent = normalize(mul(u_view, vec4(wtangent, 0.0) ).xyz);
	vec3 viewBitangent = cross(viewNormal, viewTangent);
	mat3 tbn = mat3(viewTangent, viewBitangent, viewNormal);

	v_wpos = wpos;

	vec3 view = mul(u_view, vec4(wpos, 0.0) ).xyz;
	v_view = mul(view, tbn);

	v_normal    = viewNormal;
	v_tangent   = viewTangent;
	v_bitangent = viewBitangent;

	v_texcoord0 = a_texcoord0;
}
