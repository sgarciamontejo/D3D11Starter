#pragma once
#include <DirectXMath.h>
struct VertexShaderData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 view;
};

struct PixelShaderData
{
	DirectX::XMFLOAT4 colorTint;
	float time;
};

