#include "ShaderCommon.hlsl"
#include "PCUCommon.hlsl"


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
	v2f_t v2f = (v2f_t)0;

	// forward vertex input onto the next stage
	v2f.position = float4( input.position, 1.0f );
	v2f.color = input.color * TINT;
	v2f.uv = input.uv;

	float4 worldPos = float4( input.position, 1 );
	float4 modelPos = mul( MODEL, worldPos );
	float4 cameraPos = mul( VIEW, modelPos );
	float4 clipPos = mul( PROJECTION, cameraPos );

	v2f.position = clipPos;

	return v2f;
}


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	float4 color = tDiffuse.Sample( sSampler, input.uv );
	color.r = 1.f - color.r;
	color.g = 1.f - color.g;
	color.b = 1.f - color.b;
	return color * input.color;
}
