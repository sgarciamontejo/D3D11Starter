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
Texture2D OverlayTexture : register(t1); // A texture assigned to texture slot 1
SamplerState BasicSampler : register(s0); // A sampler assigned to sampler slot 0

float4 main(VertexToPixel input) : SV_TARGET
{
    input.normal = normalize(input.normal);
    input.uv = input.uv * uvScale + uvOffset;
    float3 surfaceColor = SurfaceColor.Sample(BasicSampler, input.uv).rgb;
    float3 overlayTexture = OverlayTexture.Sample(BasicSampler, input.uv).rgb;
    
    surfaceColor *= overlayTexture;
    surfaceColor *= colorTint.rgb;
    
    float3 totalLight = ambientLight * surfaceColor;
    
    // diffuse calculation
    Light dirLight = directionalLight1;
    dirLight.Direction = normalize(dirLight.Direction);
    //dir light
    float3 dirToLight = normalize(-dirLight.Direction);
    float3 dirToCam = normalize(cameraPos - input.worldPos);
    float diffuse = saturate(dot(input.normal, dirToLight));

    
    //totalLight += (surfaceColor * (diffuse + spec)) * dirLight.Intensity * dirLight.Color; // tint specular
    totalLight += (surfaceColor * diffuse + spec) * dirLight.Intensity * dirLight.Color; // dont tint specular
    
    return float4(totalLight, 1);
}