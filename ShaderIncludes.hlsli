#ifndef _GGP_SHADER_INCLUDES_ //Each .hlsli file needs a unique identifier
#define _GGP_SHADER_INCLUDES_ 
#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2
#define MAX_SPECULAR_EXPONENT   256.0f


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
    float3 worldPos : POSITION;
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
};

#endif