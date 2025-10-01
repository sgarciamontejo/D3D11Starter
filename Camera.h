#pragma once
#include "Input.h"
#include "Transform.h"
#include <DirectXMath.h>
class Camera
{
	Transform transform;
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projMatrix;

	float fov; //radians
	float nearClip;
	float farClip;
	float speed;
	float lookSpeed;
	bool isometric = false;

public:
	Camera(float aspectRatio, XMFLOAT3 position = XMFLOAT3(0,0,0));
	~Camera();
	XMFLOAT4X4 GetViewMatrix();
	XMFLOAT4X4 GetProjectionMatrix();
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float dt);
};

