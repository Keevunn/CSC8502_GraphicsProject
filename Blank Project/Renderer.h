#pragma once
#include "AnimatedModelPool.h"
#include "Animation.h"
#include "Animator.h"
#include "LampPost.h"
#include "ModelPool.h"
#include "RobotModel.h"
#include "nclgl/OGLRenderer.h"

class KittenModel;
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
	void DrawLampPosts();

	void DrawRobots();
	void DrawKittens();

	void DrawSun(Matrix4 invViewProj, float* camPos);
	void DrawSpotLights(Matrix4 invViewProj, float* camPos);
	void DrawPointLights(Matrix4 invViewProj, float* camPos);

	Matrix4 defaultProjMatrix;

	Camera* camera = nullptr;

	// Skybox
	Shader* skyboxShader = nullptr;

	GLuint cubeMap;
	Mesh* quad = nullptr;

	// Light
	Shader* sunShader = nullptr;
	Shader* pointLightShader = nullptr; 
	Shader* spotLightShader = nullptr;

	DirectionalLight* sun = nullptr;
	std::vector<Light> pointLights;
	Mesh* lightVolume = nullptr; // Sphere

	// Terrain
	Shader* concreteShader = nullptr;

	HeightMap* concreteMap = nullptr;
	std::unordered_map<std::string, GLuint> concreteTextures; // texture type, texture id

	// Models
	Shader* environmentShader = nullptr; // Fills G-buffers
	Shader* animationShader = nullptr;

	Factory* factory = nullptr;

	AnimatedModelPool<RobotModel>* robots;
	AnimatedModelPool<KittenModel>* kittens;
	ModelPool<LampPost>* lampPosts;

	// Buffers
	Shader* combineShader = nullptr;
	
	GLuint bufferFBO; // 1 depth attachment, 3 Colour attachments
	GLuint bufferDepthTex; // Depth
	GLuint bufferColourTex; // Albedo
	GLuint bufferNormalTex; // Normals
	GLuint bufferEmissiveTex; // Emission

	GLuint pointLightFBO; // 2 Colour attachments
	GLuint lightDiffuseTex;
	GLuint lightSpecularTex;

private:
	float currentTime;
};

