#include "Camera.h"

Camera::Camera(float aspectRatio, XMFLOAT3 position = XMFLOAT3(0,0,0))
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
    fov = 90;
    nearClip = 0.1f;
    farClip = 500.0f;
    XMMATRIX projection = XMMatrixPerspectiveFovLH(fov, aspectRatio, nearClip, farClip);
    XMStoreFloat4x4(&projMatrix, projection);
}

void Camera::UpdateViewMatrix()
{
    XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&transform.GetPosition()), XMLoadFloat3(&transform.GetForward()), XMLoadFloat3(&transform.GetUp()));
    XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::Update(float dt)
{
    float camSpeed = dt * speed;
    // input handling
    if (Input::KeyDown('W')) { transform.MoveRelative(XMFLOAT3(0, 0, camSpeed)); }
    if (Input::KeyDown('S')) { transform.MoveRelative(XMFLOAT3(0, 0, -camSpeed)); }
    if (Input::KeyDown('A')) { transform.MoveRelative(XMFLOAT3(-camSpeed, 0, 0)); }
    if (Input::KeyDown('D')) { transform.MoveRelative(XMFLOAT3(camSpeed, 0, 0)); }
    if (Input::KeyDown('X')) { transform.MoveAbsolute(XMFLOAT3(0, -camSpeed, 0)); }
    if (Input::KeyDown(' ')) { transform.MoveAbsolute(XMFLOAT3(0, camSpeed, 0)); }
    if (Input::KeyDown(VK_SHIFT)) { speed *= 3; }
    if (Input::KeyDown(VK_CONTROL)) { speed *= 0.1; }

    if (Input::MouseLeftDown())
    {
        //rotate
        int cursorMovementX = Input::GetMouseXDelta();
        int cursorMovementY = Input::GetMouseYDelta();
        transform.Rotate(cursorMovementY, cursorMovementX, 0);

        //clamp
        XMFLOAT3 rotation = transform.GetPitchYawRoll();
        if (rotation.x > XM_PIDIV2) rotation.x = XM_PIDIV2;
        if (rotation.x < -XM_PIDIV2) rotation.x = -XM_PIDIV2;
        transform.SetRotation(rotation);
    }

    UpdateViewMatrix();
}
