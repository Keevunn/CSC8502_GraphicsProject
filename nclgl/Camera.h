#pragma once
#include "Matrix4.h"
#include "Vector3.h"

class Camera {
public:
	Camera(void) {
		yaw = 0.0f;
		pitch = 0.0f;
	}

	Camera(float pitch, float yaw, Vector3 position, float speed = 30.0f) : pitch(pitch), yaw(yaw), position(position), speed(speed) {}
	~Camera(void) = default;

	void UpdateCamera(float dt = 1.0f);

	Matrix4 BuildViewMatrix() const;

	float GetPitch() const { return pitch; }
	void SetPitch(float p) { pitch = p; }

	float GetYaw() const { return yaw; }
	void SetYaw(float y) { yaw = y; }

	Vector3 GetPosition() const { return position; }
	void SetPosition(Vector3 val) { position = val; }

	float GetSpeed() const { return speed; }
	void SetSpeed(float val) { speed = val; }

protected:
	float pitch;
	float yaw;
	Vector3 position;
	float speed;
};

