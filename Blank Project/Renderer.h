#pragma once
#include "Model.h"
#include "nclgl/OGLRenderer.h"

class Camera;
class Model;
class MeshAnimation;
class MeshMaterial;
class SceneNode;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

	void SetScale(float s) const { model->SetModelScale(Vector3(s)); }

protected:
	void DrawNode(SceneNode* n);

	Camera* camera;
	Model* model;
	Shader* shader;
	vector<GLuint> matTextures;
};

