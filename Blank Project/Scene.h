#pragma once
#include <glad/glad.h>

class HeightMap;
class DirectionalLight;
class Camera;
class SceneNode;

class Scene {
public:
	Scene() = default;
	virtual ~Scene();

	virtual void Initialise() = 0;
	virtual void Update();

private:
	SceneNode* root = nullptr;

	Camera* camera = nullptr;
	DirectionalLight* sun = nullptr;
	GLuint skybox = 0;
	HeightMap* terrain = nullptr;

};

