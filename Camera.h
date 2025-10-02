#pragma once
#include "Input.h"
#include "Transform.h"
#include <DirectXMath.h>
class Camera
{
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projMatrix;

public:
	Transform transform;
	float fov = XM_PIDIV4; //radians
	float nearClip;
	float farClip;
	float speed = 5.0f;
	float lookSpeed = 0.002f;
	bool isometric = false;

	Camera(float aspectRatio, XMFLOAT3 position = XMFLOAT3(0, 0, 0), float fov = XM_PIDIV4);
	~Camera();
	XMFLOAT4X4 GetViewMatrix();
	XMFLOAT4X4 GetProjectionMatrix();
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float dt);
};

