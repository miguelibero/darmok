$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_color0

#include <bgfx_shader.sh>

uniform vec4 u_rmluiData;
#define u_rmluiForceDepth	((int(u_rmluiData.x) & 1 << 0) != 0)
#define u_rmluiDepth 		u_rmluiData.y

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position.xy, 0.0, 1.0));
	if(u_rmluiForceDepth) // avoid precision problems
	{
		gl_Position.z = u_rmluiDepth;
	}
	v_color0    = a_color0;
#ifndef DARMOK_VARIANT_TEXTURE_DISABLE
	v_texcoord0 = a_texcoord0;
#endif
}