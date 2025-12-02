#include "Scene.h"

#include <algorithm>

#include "nclgl/Camera.h"
#include "nclgl/DirectionalLight.h"
#include "nclgl/SceneNode.h"

Scene::~Scene() {
	delete camera; 
	delete sun;
}

void Scene::Update(float dt) {
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_RETURN)) {
		usingCinematicCam = !usingCinematicCam;
		pathTime = 0;
	}

	if (usingCinematicCam) UpdateCinematicCam(dt);
	else camera->UpdateCamera(dt);
}

void Scene::UpdateCinematicCam(float dt) {
	//pathTime += dt * camera->GetSpeed() / 5; // slower for cinematic cam

	int currentPoint = static_cast<int>(pathTime);
	int numPoints = static_cast<int>(cameraPath.size());
	if (currentPoint >= numPoints) {
		pathTime -= numPoints;
		currentPoint = 0;
	}
	const CameraWaypoint& p1 = cameraPath.at(currentPoint);

	float duration = p1.timeOnSegment;
	duration = std::max(duration, 0.1f);
	pathTime += dt / duration;

	int ptr0 = (currentPoint - 1 + numPoints) % numPoints;
	int ptr2 = (currentPoint + 1 + numPoints) % numPoints;
	int ptr3 = (currentPoint + 2 + numPoints) % numPoints;

	const CameraWaypoint& p0 = cameraPath.at(ptr0);
	const CameraWaypoint& p2 = cameraPath.at(ptr2);
	const CameraWaypoint& p3 = cameraPath.at(ptr3);

	float t = pathTime - currentPoint; // range 0 - 1 for current segment
	Vector3 newPos = CatmullRom(p0.position, p1.position, p2.position, p3.position, t);
	camera->SetPosition(newPos);

	float pitch = std::lerp(p1.pitch, p2.pitch, t);
	camera->SetPitch(pitch);

	float yawDiff = p2.yaw - p1.yaw;
	if (yawDiff > 180) yawDiff -= 360;
	if (yawDiff < -180) yawDiff += 360;
	float yaw = p1.yaw + (yawDiff * t);

	camera->SetYaw(yaw);
}

// Code adapted from google gemini suggestions
Vector3 Scene::CatmullRom(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3 , float t) const {
	float t2 = t * t;
	float t3 = t2 * t;

	Vector3 a = p1 * 2;
	Vector3 b = p2 - p0;
	Vector3 c = (p0 * 2) - (p1 * 5) + (p2 * 4) - p3;
	Vector3 d = (p0 * -1) + (p1 * 3) - (p2 * 3) + p3;

	return (a + (b * t) + (c * t2) + (d * t3)) * 0.5f;
}

