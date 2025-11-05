#include "ShaderIncludes.hlsli"

// Constant Buffer
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float roughness;
    float3 cameraPos;
    float2 uvScale;
    float2 uvOffset;
    float3 ambientLight;
    Light directionalLight1;
}

// Example Texture2D and SamplerState definitions in an HLSL pixel shader
Texture2D SurfaceColor : register(t0); // A texture assigned to texture slot 0
SamplerState BasicSampler : register(s0); // A sampler assigned to sampler slot 0

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    input.normal = normalize(input.normal);
    input.uv = input.uv * uvScale + uvOffset;
    float3 surfaceColor = SurfaceColor.Sample(BasicSampler, input.uv).rgb;
    surfaceColor *= colorTint.rgb;
    
    float3 totalLight = ambientLight * surfaceColor;
    
    // diffuse calculation
    Light dirLight = directionalLight1;
    dirLight.Direction = normalize(dirLight.Direction);
    //dir light
    float3 dirToLight = normalize(-dirLight.Direction);
    float3 dirToCam = normalize(cameraPos - input.worldPos);
    float diffuse = saturate(dot(input.normal, dirToLight));
    
    totalLight += (diffuse * surfaceColor) * dirLight.Intensity * dirLight.Color;
    
    //directionalLight1.Color
    
    return float4(totalLight, 1);
    //return float4(input.normal, 1);
}