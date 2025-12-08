#include "ShaderIncludes.hlsli"

// Constant Buffer for external data
cbuffer ExternalData : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

// simplified vertex shader for rendering to a shadow map
float4 main(VertexShaderInput input) : SV_POSITION
{
    matrix wvp = mul(projection, mul(view, world));
    return mul(wvp, float4(input.localPosition, 1.0f));
}