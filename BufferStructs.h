#pragma once
#include <DirectXMath.h>
#include "Lights.h"
struct VertexShaderData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldInvTranspose;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 view;
};

struct PixelShaderData
{
	Light lights[5];

	int lightCount;
	DirectX::XMFLOAT3 ambientLight;

	DirectX::XMFLOAT4 colorTint;

	float roughness;
	DirectX::XMFLOAT3 cameraPos;

	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;

};

