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
Texture2D Albedo : register(t0); // A texture assigned to texture slot 0
Texture2D NormalMap : register(t1); // Normals texture slot 1
Texture2D RoughnessMap : register(t2); // Roughness texture slot 2
Texture2D MetalnessMap : register(t3); // Metallness texture slot 3

TextureCube EnvironmentMap : register(t4);
Texture2D ShadowMap : register(t5);

SamplerState BasicSampler : register(s0); // A sampler assigned to sampler slot 0
SamplerComparisonState ShadowSampler : register(s1); // shadow sampler slot 1

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
    // perspective divide
    //input.shadowMapPos /= input.shadowMapPos.w;
    
    // convert normalized device coordinates to UVs for sampling
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y; // flip y
    
    //grab distances needed: light to pixel and closest surface
    float distToLight = input.shadowMapPos.z;
    float distShadowMap = ShadowMap.Sample(BasicSampler, shadowUV).r;
    
    // ratio of comparison results using SampleCmpLevelZero()
    float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, distToLight).r;
    
    // for testing, return black
//    if (distShadowMap < distToLight)
//        return float4(0, 0, 0, 1);
    
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    input.uv = input.uv * uvScale + uvOffset;
    float3 surfaceColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2);
    surfaceColor *= colorTint.rgb;
    
    // unpack normal map
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
    float3 B = cross(T, N);
    
    float3x3 TBN = float3x3(T, B, N); // convert to world space
    input.normal = normalize(mul(unpackedNormal, TBN));

    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;
    //float specularScale = SpecularMap.Sample(BasicSampler, input.uv).r;
    
    // Specular color determination -----------------
    // Assume albedo texture is actually holding specular color where metalness == 1
    // Note the use of lerp here - metal is generally 0 or 1, but might be in between
    // because of linear texture sampling, so we lerp the specular color to match
    float3 specColor = lerp(0.04f, surfaceColor.rgb, metalness);
    
    float3 totalLight = ambientLight * surfaceColor;
    //float3 totalLight = surfaceColor;
   
    // diffuse calculation
    for (int i = 0; i < lightCount; i++)
    {
        Light light = lights[i];
        light.Direction = normalize(light.Direction);

        // directional light
        if (light.Type == 0)
        {
            float3 lightResult = DirectionalLight(light, input.normal, input.worldPos, cameraPos, roughness, metalness, surfaceColor, specColor);
            
            if (i == 0)
            {
                lightResult *= shadowAmount;
            }
            
            totalLight += lightResult;
        }
        // point light
        else if (light.Type == 1)
        {
            totalLight += PointLight(light, input.normal, input.worldPos, cameraPos, roughness, metalness, surfaceColor, specColor);
        }
        else if (light.Type == 2)
        {
            totalLight += SpotLight(light, input.normal, input.worldPos, cameraPos, roughness, metalness, surfaceColor, specColor);
        }

    }
    
    // Sample environment map using reflected view vector
    float3 viewVector = normalize(cameraPos - input.worldPos);
    float3 reflectionVector = reflect(-viewVector, input.normal); // Cam to pixel vector (negate)
    float3 reflectionColor = EnvironmentMap.Sample(BasicSampler, reflectionVector).rgb;
    
    float3 finalColor = lerp(totalLight, reflectionColor, F_Schlick(input.normal, viewVector, F0_NON_METAL));
    
    //totalLight += (surfaceColor * (diffuse + spec)) * dirLight.Intensity * dirLight.Color; // tint specular
    //totalLight += (surfaceColor * diffuse + spec) * dirLight.Intensity * dirLight.Color; // dont tint specular
    
    return float4(pow(finalColor, 1 / 2.2), 1); // gamma correction
    //return float4(input.normal, 1);
}