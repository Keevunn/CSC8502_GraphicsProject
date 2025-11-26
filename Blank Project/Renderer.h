#pragma once
#include "Animation.h"
#include "Animator.h"
#include "RobotModel.h"
#include "nclgl/OGLRenderer.h"

class Factory;
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

	void SetTime(float t) { currentTime = t; }

protected:
	void GenerateScreenTexture(GLuint& into, bool depth = false); // Makes a new texture
	void FillBuffers(); // G-Buffer fill render pass 
	void DrawLights(); // Lighting render pass
	void CombineBuffers(); // Combination render pass

	void DrawSkybox();

	void DrawConcreteFloor();
	void DrawNode(SceneNode* n);

	void DrawFactory();

	void DrawRobot();
	//void DrawRobotNode(SceneNode* n);
	void DrawRobotEmissive();

	void DrawSun(Matrix4 invViewProj, float* camPos);
	void DrawPointLights(Matrix4 invViewProj, float* camPos);

	Matrix4 defaultProjMatrix;

	Camera* camera = nullptr;

	Shader* robotShader = nullptr;
	Shader* emissiveShader = nullptr;
	RobotModel* robot = nullptr;
	
	//Animation* animation = nullptr; 
	Animator* animator = nullptr;

	DirectionalLight* sun = nullptr;
	Shader* sunShader;

	Shader* skyboxShader = nullptr;
	Mesh* quad = nullptr;
	GLuint cubeMap;

	Shader* environmentShader = nullptr; // Fills G-buffers
	Factory* factory = nullptr;
	std::vector<Light> pointLights;
	Mesh* lightVolume = nullptr; // Sphere

	Shader* pointLightShader = nullptr; // Calculates lighting
	Shader* combineShader = nullptr;

	Shader* concreteShader = nullptr;
	HeightMap* concreteMap = nullptr;
	std::unordered_map<std::string, GLuint> concreteTextures; // texture type, texture id

	GLuint bufferFBO;
	GLuint bufferColourTex; // Albedo
	GLuint bufferNormalTex; // Normals
	GLuint bufferDepthTex; // Depth
	GLuint bufferEmissiveTex; // Emission

	GLuint pointLightFBO;
	GLuint lightDiffuseTex;
	GLuint lightSpecularTex;

private:
	float currentTime;
};

