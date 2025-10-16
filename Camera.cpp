#include "Camera.h"

Camera::Camera(float aspectRatio, XMFLOAT3 position, float fov)
{
    transform.SetPosition(position);

    UpdateViewMatrix();
    UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{
}

XMFLOAT4X4 Camera::GetViewMatrix()
{
    return viewMatrix;
}

XMFLOAT4X4 Camera::GetProjectionMatrix()
{
    return projMatrix;
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
    if (isometric) { return; }

    //perspective
    nearClip = 0.01f;
    farClip = 1000.0f;
    XMMATRIX projection = XMMatrixPerspectiveFovLH(fov, aspectRatio, nearClip, farClip);
    XMStoreFloat4x4(&projMatrix, projection);
}

void Camera::UpdateViewMatrix()
{
    XMFLOAT3 pos = transform.GetPosition();
    XMFLOAT3 forward = transform.GetForward();
    XMFLOAT3 up = transform.GetUp();
    XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&pos), XMLoadFloat3(&forward), XMLoadFloat3(&up));
    XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::Update(float dt)
{
    float camSpeed = dt * speed;
    // input handling
    if (Input::KeyDown(VK_SHIFT)) { camSpeed *= 3; }
    if (Input::KeyDown(VK_CONTROL)) { camSpeed *= 0.1; }
    if (Input::KeyDown('W')) { transform.MoveRelative(XMFLOAT3(0, 0, camSpeed)); }
    if (Input::KeyDown('S')) { transform.MoveRelative(XMFLOAT3(0, 0, -camSpeed)); }
    if (Input::KeyDown('A')) { transform.MoveRelative(XMFLOAT3(-camSpeed, 0, 0)); }
    if (Input::KeyDown('D')) { transform.MoveRelative(XMFLOAT3(camSpeed, 0, 0)); }
    if (Input::KeyDown('X')) { transform.MoveAbsolute(XMFLOAT3(0, -camSpeed, 0)); }
    if (Input::KeyDown(' ')) { transform.MoveAbsolute(XMFLOAT3(0, camSpeed, 0)); }

    if (Input::MouseLeftDown())
    {
        //rotate
        float cursorMovementX = lookSpeed * Input::GetMouseXDelta();
        float cursorMovementY = lookSpeed * Input::GetMouseYDelta();
        transform.Rotate(cursorMovementY, cursorMovementX, 0);

        //clamp
        XMFLOAT3 rotation = transform.GetPitchYawRoll();
        if (rotation.x > XM_PIDIV2) rotation.x = XM_PIDIV2;
        if (rotation.x < -XM_PIDIV2) rotation.x = -XM_PIDIV2;
        transform.SetRotation(rotation);
    }

    UpdateViewMatrix();
}
