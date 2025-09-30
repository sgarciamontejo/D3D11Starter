#include "Transform.h"

Transform::Transform() : 
	position(0, 0, 0), 
	rotation(0, 0, 0), 
	scale(1, 1, 1)
{
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
}

Transform::~Transform() {

}

// Setter Definitions
void Transform::SetPosition(float x, float y, float z) {
	this->position = XMFLOAT3(x, y, z);
}

void Transform::SetPosition(XMFLOAT3 position) {
	this->position = position;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	this->rotation = XMFLOAT3(pitch, yaw, roll);
}

void Transform::SetRotation(XMFLOAT3 rotation)
{
	this->rotation = rotation;
}

void Transform::SetScale(float x, float y, float z)
{
	this->scale = XMFLOAT3(x, y, z);
}

void Transform::SetScale(XMFLOAT3 scale)
{
	this->scale = scale;
}

// Getters
XMFLOAT3 Transform::GetPosition()
{
	return position;
}

XMFLOAT3 Transform::GetPitchYawRoll()
{
	return rotation;
}

XMFLOAT3 Transform::GetScale()
{
	return scale;
}

XMFLOAT4X4 Transform::GetWorldMatrix()
{
	XMMATRIX transform = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX scaling = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX rotate = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	// Scale * Rotation * Transformation
	XMMATRIX worldMatrix = scaling * rotate * transform;
	XMStoreFloat4x4(&world, worldMatrix);
	XMStoreFloat4x4(&worldInverseTranspose,
		XMMatrixInverse(0, XMMatrixTranspose(worldMatrix)));

	return world;
}

XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	return XMFLOAT4X4();
}

// Movers
void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
}

void Transform::MoveAbsolute(XMFLOAT3 offset)
{
	position.x += offset.x;
	position.y += offset.y;
	position.z += offset.z;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	rotation.x += pitch;
	rotation.y += yaw;
	rotation.z += roll;
}

void Transform::Rotate(XMFLOAT3 rotation)
{
	this->rotation.x += rotation.x;
	this->rotation.y += rotation.y;
	this->rotation.z += rotation.z;
}

void Transform::Scale(float x, float y, float z)
{
	scale.x += x;
	scale.y += y;
	scale.z += z;
}

void Transform::Scale(XMFLOAT3 scale)
{
	this->scale.x += scale.x;
	this->scale.y += scale.y;
	this->scale.z += scale.z;
}

void Transform::MoveRelative(float x, float y, float z)
{
}

void Transform::MoveRelative(XMFLOAT3 offset)
{
}
