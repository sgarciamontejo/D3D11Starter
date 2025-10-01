#pragma once
#include <DirectXMath.h>
struct VertexShaderData
{
	XMFLOAT4 colorTint;
	XMFLOAT4X4 world;
	XMFLOAT4X4 projection;
	XMFLOAT4X4 view;
};

