#pragma once
#include "Animator.h"
#include "Model.h"
#include "nclgl/OGLRenderer.h"

class HeightMap;
class Camera;
class MeshAnimation;
class MeshMaterial;
class SceneNode;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

	void SetScale(float s) const { model->SetModelScale(Vector3(s)); } // TODO Remove soon!!!

protected:
	void DrawNode(SceneNode* n);

	void DrawHeightMap();
	void DrawSkybox();

	Camera* camera = nullptr;

	Shader* modelShader = nullptr;
	Model* model = nullptr;
	GLuint robotTexture;
	
	Animation* animation = nullptr; // Might change to a vector of animations later
	Animator* animator = nullptr;

	Shader* skyboxShader = nullptr;
	Mesh* quad = nullptr;
	GLuint cubeMap;

	SceneNode* environmentRoot = nullptr;

	Shader* terrainShader = nullptr;
	HeightMap* heightMap = nullptr;
	std::unordered_map<std::string, GLuint> concreteTextures; // texture type, texture id

};

