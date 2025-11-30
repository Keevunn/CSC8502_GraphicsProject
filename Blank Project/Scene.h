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

class Scene {
public:
	Scene() = default;
	virtual ~Scene();

	virtual void Initialise(float width, float height) = 0;
	virtual void Update(float dt) = 0;

	virtual void RenderGeometry(Renderer& renderer) = 0;
	virtual void RenderLights(Renderer& renderer) = 0;

	Matrix4 GetDefaultProjMatrix() const { return defaultProjMatrix; }

	Camera* GetCamera() const { return camera; }
	DirectionalLight* GetSun() const { return sun; }
	GLuint GetSkyboxTex() const { return skybox; }
	

protected:
	Matrix4 defaultProjMatrix;

	Camera* camera = nullptr;
	DirectionalLight* sun = nullptr;
	GLuint skybox = 0;

};

