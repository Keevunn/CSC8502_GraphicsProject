#include "Camera.h"
#include "Window.h"
#include <algorithm>

void Camera::UpdateCamera(float dt) {
	pitch -= Window::GetMouse()->GetRelativePosition().y;
	yaw -= Window::GetMouse()->GetRelativePosition().x;

	// clamp pitch to [-90,90]
	pitch = std::clamp(pitch, -90.0f, 90.0f);

	if (yaw < 0)
		yaw += 360.0f;
	if (yaw > 360.0f)
		yaw -= 360.0f;

	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));
	Vector3 forward = rotation * Vector3(0, 0, -1); // Down the negative z axis
	Vector3 right = rotation * Vector3(1, 0, 0); // Towards the right

	float speed = 30.0f * dt; // 30 units per sec

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W))
		position += forward * speed;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S))
		position -= forward * speed;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A))
		position -= right * speed;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D))
		position += right * speed;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) // Tutorial notes used shift
		position.y += speed;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_CONTROL)) // Tutorial notes used space
		position.y -= speed;
}

Matrix4 Camera::BuildViewMatrix() const {
	return	Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) * // Rotations must come first so the camera isn't rotated at a distance from the origin
			Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
			Matrix4::Translation(-position); // Cheap way of computing the inverse of view matrix
}
