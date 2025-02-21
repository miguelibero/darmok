$input v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <darmok_material_basic.sc>

void main()
{
	Material mat = getMaterial(v_texcoord0);
	gl_FragColor = mat.diffuse * v_color0;
}