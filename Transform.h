// edited 9/24/25
#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class Transform
{
	XMFLOAT3 position;
	XMFLOAT3 rotation; // pitch / yaw / angles
	XMFLOAT3 scale;
	XMFLOAT4X4 world;
	XMFLOAT4X4 worldInverseTranspose; // used in a future assignment

public:
	Transform(); // Constructor
	~Transform();

	// Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 position);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(XMFLOAT3 rotation);
	void SetScale(float x, float y, float z);
	void SetScale(XMFLOAT3 scale);

	// Getters
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetPitchYawRoll();
	XMFLOAT3 GetScale();
	XMFLOAT4X4 GetWorldMatrix();
	XMFLOAT4X4 GetWorldInverseTransposeMatrix();

	// Transformers
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(XMFLOAT3 offset);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(XMFLOAT3 rotation);
	void Scale(float x, float y, float z);
	void Scale(XMFLOAT3 scale);
};

