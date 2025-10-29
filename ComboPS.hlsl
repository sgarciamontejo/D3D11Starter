struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

// Constant Buffer
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 uvScale;
    float2 uvOffset;
}

// Example Texture2D and SamplerState definitions in an HLSL pixel shader
Texture2D SurfaceColor : register(t0); // A texture assigned to texture slot 0
Texture2D OverlayTexture : register(t1); // A texture assigned to texture slot 1
SamplerState BasicSampler : register(s0); // A sampler assigned to sampler slot 0

float4 main(VertexToPixel input) : SV_TARGET
{
    input.uv = input.uv * uvScale + uvOffset;
    float3 surfaceColor = SurfaceColor.Sample(BasicSampler, input.uv).rgb;
    float3 overlayTexture = OverlayTexture.Sample(BasicSampler, input.uv).rgb;
    
    surfaceColor *= overlayTexture;
    surfaceColor *= colorTint;
    return float4(surfaceColor, 1);
}