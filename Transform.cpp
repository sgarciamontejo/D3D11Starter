#include "Transform.h"

Transform::Transform() : 
	position(0, 0, 0), 
	rotation(0, 0, 0), 
	scale(1, 1, 1)
{
	DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&worldInverseTranspose, DirectX::XMMatrixIdentity());
}

// Setter Definitions
void Transform::SetPosition(float x, float y, float z) {
	
}