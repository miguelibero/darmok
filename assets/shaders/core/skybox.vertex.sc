$input a_position
$output v_texcoord0

#include <bgfx_shader.sh>

void main()
{
	// remove view translation
	mat4 view = u_view;
	
#if BGFX_SHADER_LANGUAGE_GLSL	
	view[3][0] = 0.0;
	view[3][1] = 0.0;
	view[3][2] = 0.0;
#else
	view[0][3] = 0.0;
	view[1][3] = 0.0;
	view[2][3] = 0.0;
#endif

	mat4 viewProj = mul(u_proj, view);
	vec4 pos = mul(viewProj, vec4(a_position, 1.0));
	gl_Position = pos.xyww;
	v_texcoord0 = a_position;
}