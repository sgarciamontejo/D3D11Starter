#ifndef _GGP_SHADER_INCLUDES_ //Each .hlsli file needs a unique identifier
#define _GGP_SHADER_INCLUDES_ 
#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2
#define MAX_SPECULAR_EXPONENT   256.0f
#define MIN_ROUGHNESS           0.0000001
#define PI                      3.14159265359f


// structs and funcs definitions

// Light struct
struct Light
{
    int Type; // enum types for Light | directional - 0 / point - 1 / spot - 2
    float3 Direction; // light direction
    float Range; //max range for attenuation (point and spot lights)
    float3 Position; // point and spot lights position in space
    float Intensity;
    float3 Color;
    float SpotInnerAngle; // inner cone angle (radians)
    float SpotOuterAngle; // outer cone angle (radians) aka penumbra
    float2 Padding; // 16 byte boundary
};

static const float F0_NON_METAL = 0.04f;

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
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
    float3 tangent : TANGENT;
    float3 worldPos : POSITION;
};

// redefine V to P struct
struct VertexToPixel_Sky
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition : POSITION; // XYZ position
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};


// PBR Lighting Calculations
// Cook-Terrence BRDF
// spec(v,l) = D(n,h,a)F(v,h,f0)G(n,v,l,a) / 4(n * v)(n * l)
//
// Normal Distribution - Trowbridge-Reitz (GGX)
float D_GGX(float3 n, float3 h, float roughness)
{
    // Pre-calculations
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness; // Remapping roughness
    float a2 = max(a * a, MIN_ROUGHNESS);
    // Denominator to be squared is ((n dot h)^2 * (a^2 - 1) + 1)
    float denomToSquare = NdotH2 * (a2 - 1) + 1;
    return a2 / (PI * denomToSquare * denomToSquare);
}

// Geometric Shawing - Schlick GGX
float G_SchlickGGX(float3 n, float3 v, float roughness)
{
    float k = pow(roughness + 1, 2) / 8.0f; // End result of remaps
    float NdotV = saturate(dot(n, v));
    return 1 / (NdotV * (1 - k) + k);
}

// Fresnel - Schlick's Approximation
// n = normal
// v = view vector
// f0 = specular value (0.04 for nonmetal)
// F(n, v, f0) = f0 + (1-f0)(1 - (n dot v))^5
float3 F_Schlick(float3 v, float3 h, float3 f0)
{
    float VdotH = saturate(dot(v, h));
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Microfacet calculation
// n = normal
// l = light direction
// v = cam direction
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0)
{
    float3 h = normalize(v + l);
    // Run each function: D and G are scalars, F is a vector
    float D = D_GGX(n, h, roughness);
    float3 F = F_Schlick(v, h, f0);
    float G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);
    // Final formula
    return (D * F * G) / 4;
}

float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}

//-----------------------

float SpecularPhong(float3 normal, float3 lightDir, float3 camDir, float roughness)
{
    float3 reflective = reflect(-lightDir, normal);
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    float spec = pow(max(dot(reflective, camDir), 0.0f), specExponent);
    if (specExponent < 0.05f)
    {
        spec = 0;
    }

    return spec;
}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

float Diffuse(float3 normal, float3 lightDir)
{
    return saturate(dot(normal, lightDir));
}

float3 DirectionalLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float metalness, float3 surfaceColor, float3 specColor)
{
    float3 dirToLight = normalize(-light.Direction);
    float3 dirToCam = normalize(camPos - worldPos);
    
    float diffuse = Diffuse(normal, dirToLight);
    float3 spec = MicrofacetBRDF(normal, dirToLight, dirToCam, roughness, specColor);
    //float spec = SpecularPhong(normal, dirToLight, dirToCam, roughness) * any(diffuse);

    // Diffuse with Energy conservation - Include cutting diffuse for metals
    float3 h = normalize(dirToCam + dirToLight);
    float3 F = F_Schlick(dirToCam, h, specColor);
    float3 balancedDiff = DiffuseEnergyConserve(diffuse, F, metalness);
    
    return (surfaceColor * (balancedDiff + spec)) * light.Intensity * light.Color;
}

float3 PointLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float metalness, float3 surfaceColor, float3 specColor)
{
    //light pos - pixel world pos
    float3 dirToLight = normalize(light.Position - worldPos);
    float3 dirToCam = normalize(camPos - worldPos);
    
    float diffuse = Diffuse(normal, dirToLight);
    float3 spec = MicrofacetBRDF(normal, dirToLight, dirToCam, roughness, specColor);
    //float spec = SpecularPhong(normal, dirToLight, dirToCam, roughness) * any(diffuse);

    // Diffuse with Energy conservation - Include cutting diffuse for metals
    float3 h = normalize(dirToCam + dirToLight);
    float3 F = F_Schlick(dirToCam, h, specColor);
    float3 balancedDiff = DiffuseEnergyConserve(diffuse, F, metalness);
    
    float attenuate = Attenuate(light, worldPos);
    
    return (surfaceColor * (balancedDiff + spec)) * attenuate * light.Intensity * light.Color;
}

float3 SpotLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float metalness, float3 surfaceColor, float3 specColor)
{
    float3 dirToLight = normalize(light.Position - worldPos);
    
    // get cos(angle) between pixel and light dir
    float pixelAngle = saturate(dot(-dirToLight, light.Direction));
    
    // get cosibes of angles and calc range
    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    float falloffRange = cosOuter - cosInner;
    
    // linear falloff over range, clamp 0-1 and apply to light calc
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
    return PointLight(light, normal, worldPos, camPos, roughness, metalness, surfaceColor, specColor) * spotTerm;
}

#endif