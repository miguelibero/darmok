$input v_color0, v_texcoord0

#include <darmok.include.sc>

SAMPLER2D(s_texColor, 0);

#if LOD_ENABLED
uniform vec4 u_imageLodEnabled;
#define u_imageLod     u_imageLodEnabled.x
#define u_imageEnabled u_imageLodEnabled.y
#endif

void main()
{
#if LOD_ENABLED	
	vec3 color = texture2DLod(s_texColor, v_texcoord0, u_imageLod).xyz;
	float alpha = 0.2 + 0.8 * u_imageEnabled;
	gl_FragColor = vec4(color, alpha) * v_color0;
#else
	vec4 texel = texture2D(s_texColor, v_texcoord0);
	gl_FragColor = texel * v_color0;
#endif
}