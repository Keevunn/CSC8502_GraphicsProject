#pragma once
#include "nclgl/OGLRenderer.h"

class Renderer;
class Matrix4;
class Shader;
class HeightMap;
class Camera;
class SceneNode;

#define MODELSDIR "../Models/"
#define ANIMATIONDIR "../Animations/"

struct CameraWaypoint {
	Vector3 position;
	float pitch;
	float yaw;
	float timeOnSegment;
	CameraWaypoint(Vector3 position, float pitch, float yaw, float timeOnSegment = 5.0f)
		: position(position), pitch(pitch), yaw(yaw), timeOnSegment(timeOnSegment) {}
};

class Scene {
public:
	Scene() = default;
	virtual ~Scene();

	virtual void Initialise(Renderer& renderer) = 0;

	virtual void Update(float dt);
	void UpdateCinematicCam(float dt);

	virtual void RenderGeometry(Renderer& renderer) = 0;
	virtual void RenderLights(Renderer& renderer) = 0;

	Matrix4 GetDefaultProjMatrix() const { return defaultProjMatrix; }

	Camera* GetCamera() const { return camera; }
	Vector3 CatmullRom(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t) const;

	DirectionalLight* GetSun() const { return sun; }
	GLuint GetSkyboxTex() const { return skybox; }

	float GetFogDensity() const { return fogDensity; }
	
	void ResetPathTime() { pathTime = 0; }
	bool LoadSuccess() const { return init; }

protected:
	bool init; // Used to check if finished initialisation
	Matrix4 defaultProjMatrix;

	Camera* camera = nullptr;
	std::vector<CameraWaypoint> cameraPath;
	bool usingCinematicCam = true;
	float pathTime = 0.0f;

	DirectionalLight* sun = nullptr;
	GLuint skybox = 0;

	float fogDensity = 0.5f;

};

