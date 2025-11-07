#include "ShaderIncludes.hlsli"

// Constant Buffer
cbuffer ExternalData : register(b0)
{
    Light lights[5]; // 5 lights
    int lightCount;
    float3 ambientLight;
    float4 colorTint;
    float roughness;
    float3 cameraPos;
    float2 uvScale;
    float2 uvOffset;
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
    for (int i = 0; i < lightCount; i++)
    {
        Light light = lights[i];
        light.Direction = normalize(light.Direction);

        // directional light
        if (light.Type == 0)
        {
            totalLight += DirectionalLight(light, input.normal, input.worldPos, cameraPos, roughness, surfaceColor);
        }
        // point light
        else if (light.Type == 1)
        {
            totalLight += PointLight(light, input.normal, input.worldPos, cameraPos, roughness, surfaceColor);
        }
        else if (light.Type == 2)
        {
            totalLight += SpotLight(light, input.normal, input.worldPos, cameraPos, roughness, surfaceColor);
        }

    }
    //totalLight += (surfaceColor * (diffuse + spec)) * dirLight.Intensity * dirLight.Color; // tint specular
    //totalLight += (surfaceColor * diffuse + spec) * dirLight.Intensity * dirLight.Color; // dont tint specular
    
    return float4(totalLight, 1);
}