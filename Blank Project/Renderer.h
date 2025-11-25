#pragma once
#include "Animator.h"
#include "Model.h"
#include "nclgl/OGLRenderer.h"

class Environment;
class HeightMap;
class Camera;
class MeshAnimation;
class MeshMaterial;
class SceneNode;
class Light;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

	void SetScale(float s) const { model->SetModelScale(Vector3(s)); } // TODO Remove soon!!!

protected:
	void GenerateScreenTexture(GLuint& into, bool depth = false); // Makes a new texture
	void FillBuffers(); // G-Buffer fill render pass 
	void CombineBuffers(); // Combination render pass

	void DrawSkybox();

	void DrawHeightMap();
	void DrawNode(SceneNode* n, Shader* s);

	void DrawSun();
	void DrawPointLights(); // Lighting render pass

	Camera* camera = nullptr;

	Shader* modelShader = nullptr;
	Model* model = nullptr;
	GLuint robotTexture;
	
	Animation* animation = nullptr; // Might change to a vector of animations later
	Animator* animator = nullptr;

	DirectionalLight* sun = nullptr;
	Shader* sunShader;

	Shader* skyboxShader = nullptr;
	Mesh* quad = nullptr;
	GLuint cubeMap;

	Shader* environmentShader = nullptr; // Fills G-buffers
	Environment* environment = nullptr;
	std::vector<Light> pointLights;
	Mesh* lightVolume = nullptr; // Sphere

	Shader* pointLightShader = nullptr; // Calculates lighting
	Shader* combineShader = nullptr;

	//Shader* terrainShader = nullptr;
	HeightMap* heightMap = nullptr;
	std::unordered_map<std::string, GLuint> concreteTextures; // texture type, texture id

	GLuint bufferFBO;
	GLuint bufferColourTex; // Albedo
	GLuint bufferNormalTex; // Normals
	GLuint bufferDepthTex; // Depth

	GLuint pointLightFBO;
	GLuint lightDiffuseTex;
	GLuint lightSpecularTex;
};

