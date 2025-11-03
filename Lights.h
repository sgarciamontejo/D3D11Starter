#pragma once
#include <DirectXMath.h>
#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

struct Light
{
	int Type;	// enum types for Light | directional - 0 / point - 1 / spot - 2
	DirectX::XMFLOAT3 Direction; // light direction
	float Range; //max range for attenuation (point and spot lights)
	DirectX::XMFLOAT3 Position; // point and spot lights position in space
	float Intensity;
	DirectX::XMFLOAT3 Color;
	float SpotInnerAngle; // inner cone angle (radians)
	float SpotOuterAngle; // outer cone angle (radians) aka penumbra
	DirectX::XMFLOAT2 Padding; // 16 byte boundary
};