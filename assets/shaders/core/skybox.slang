
uniform float4x4 u_view;
uniform float4x4 u_proj;

struct VSInput
{
    float3 a_position : POSITION;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 v_texcoord0 : TEXCOORD0;
};

[[vk::binding(1, 0)]]
uniform TextureCube<float4> s_texColor : register(t1);

[[vk::binding(1, 0)]]
uniform SamplerState s_texColorState : register(s1);

[shader("vertex")]
VSOutput vert(VSInput IN)
{    
    VSOutput OUT;

	float4x4 view = u_view;
	
#ifdef DARMOK_GLSL	
	view[3][0] = 0.0;
	view[3][1] = 0.0;
	view[3][2] = 0.0;
#else
	view[0][3] = 0.0;
	view[1][3] = 0.0;
	view[2][3] = 0.0;
#endif

	float4x4 viewProj = mul(u_proj, view);
	float4 pos = mul(viewProj, float4(IN.a_position, 1.0));

	OUT.position = pos.xyww;
	OUT.v_texcoord0 = IN.a_position;

    return OUT;
}

[shader("fragment")]
float4 frag(VSOutput IN) : SV_Target
{
    return s_texColor.Sample(s_texColorState, IN.v_texcoord0);
}

