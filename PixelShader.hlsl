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
    Light lights[5]; // 5 lights
    int lightCount;
}

// Example Texture2D and SamplerState definitions in an HLSL pixel shader
Texture2D SurfaceColor : register(t0); // A texture assigned to texture slot 0
Texture2D SpecularMap : register(t1);

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
    
    float specularScale = SpecularMap.Sample(BasicSampler, input.uv).r;
    
    float3 totalLight = ambientLight * surfaceColor;
    
    // diffuse calculation
    for (int i = 0; i < lightCount; i++)
    {
        Light light = lights[i];
        light.Direction = normalize(light.Direction);
        
        // directional light
        if (light.Type == 0)
        {
            totalLight += DirectionalLight(light, input.normal, input.worldPos, cameraPos, roughness, surfaceColor, specularScale);
        }
        // point light
        else if (light.Type == 1)
        {
            totalLight += PointLight();
        }

    }
    
        totalLight += (surfaceColor * (diffuse + spec)) * dirLight.Intensity * dirLight.Color; // tint specular
    //totalLight += (surfaceColor * diffuse + spec) * dirLight.Intensity * dirLight.Color; // dont tint specular
    
    return float4(totalLight, 1);
    //return float4(input.normal, 1);
}