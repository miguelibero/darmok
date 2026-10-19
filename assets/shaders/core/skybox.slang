#include <darmok.inc.slang>

struct VSInput
{
    float3 a_position : POSITION;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 v_texcoord0 : TEXCOORD0;
};

[[vk::binding(0, 0)]]
uniform TextureCube<float4> s_texColor : register(t0);

[[vk::binding(0, 0)]]
uniform SamplerState s_texColorState : register(s0);

[shader("vertex")]
VSOutput vert(VSInput IN)
{    
    VSOutput OUT;

	// Strip translation: replace each row/col with its xyz and a zero w.
	// Avoid double-index element writes (view[i][j]=0) which generate SPIRV
	// that trips a spirv-cross bug — use whole-vector replacement instead.
#ifdef DARMOK_GLSL
	// Column-major path: view[c] is a column; column 3 holds translation.
	float4x4 view = float4x4(u_view[0], u_view[1], u_view[2],
	                          float4(0.0, 0.0, 0.0, u_view[3].w));
#else
	// Row-major path: view[r] is a row; rows 0-2 have translation in .w.
	float4x4 view = float4x4(float4(u_view[0].xyz, 0.0),
	                          float4(u_view[1].xyz, 0.0),
	                          float4(u_view[2].xyz, 0.0),
	                          u_view[3]);
#endif

	float4x4 viewProj = mul(u_proj, view);
	float4 pos = mul(viewProj, float4(IN.a_position, 1.0));

	OUT.position = float4(pos.xy, pos.w, pos.w);
	OUT.v_texcoord0 = IN.a_position;

    return OUT;
}

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
    discardIfHiddenBySceneDepth(IN.position.xyz);
    return s_texColor.Sample(s_texColorState, IN.v_texcoord0);
}

