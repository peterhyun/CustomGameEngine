#pragma once

const char* defaultShaderSource = R"(
	struct vs_input_t
	{
		float3 localPosition : POSITION;
		float4 color : COLOR;
		float2 uv : TEXCOORD;
	};

	struct v2p_t
	{
		float4 position : SV_POSITION;
		float4 color : COLOR;
		float2 uv : TEXCOORD;
	};

	cbuffer CameraConstants : register(b2)
	{
		float4x4 ProjectionMatrix;
		float4x4 ViewMatrix;
	};

	cbuffer ModelConstants : register(b3)
	{
		float4x4 ModelMatrix;
		float4	 ModelColor;
	};

	Texture2D basicTexture: register(t0);
	SamplerState basicSampler: register(s0);

	v2p_t VertexMain(vs_input_t input)
	{
		float4 clipPosition = mul(ModelMatrix, float4(input.localPosition, 1));
		clipPosition = mul(ViewMatrix, clipPosition);
		clipPosition = mul(ProjectionMatrix, clipPosition);
		v2p_t v2p;
		v2p.position = clipPosition;
		v2p.color = input.color;
		v2p.uv = input.uv;

		return v2p;
	}

	float4 PixelMain(v2p_t input) : SV_Target0
	{
		return input.color * ModelColor * basicTexture.Sample(basicSampler, input.uv);
	}
)";