#pragma once
#include "AnimatedModelPool.h"
#include "LampPost.h"
#include "ModelPool.h"
#include "RobotModel.h"
#include "nclgl/OGLRenderer.h"

class FactoryScene;
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

	void DrawSkybox();
	void DrawSpotLights(std::vector<SpotLight*> lights);
	void DrawPointLights(std::vector<SpotLight*> lights);

	// Factory Scene
	void DrawConcreteFloor(HeightMap* floor, std::unordered_map<std::string, GLuint> floorTextures);
	void DrawFactory(Factory* factory);
	void DrawLampPosts(const ModelPool<LampPost>* lampPosts);
	void DrawRobots(const AnimatedModelPool<RobotModel>* robots);
	void DrawKittens(const AnimatedModelPool<KittenModel>* kittens);


protected:
	void GenerateScreenTexture(GLuint& into, bool depth = false); // Makes a new texture
	void FillBuffers(); // G-Buffer fill render pass 
	void DrawLights(); // Lighting render pass
	void CombineBuffers(); // Combination render pass

	void DrawNode(SceneNode* n);

	void DrawSun(Matrix4 invViewProj, float* camPos);

	void BindLightUniforms(GLuint programLocation, Matrix4 invViewProj, float* camPos) const;

	FactoryScene* factoryScene;

	// Skybox
	Shader* skyboxShader = nullptr;

	// Light
	Shader* sunShader = nullptr;
	Shader* pointLightShader = nullptr; 
	Shader* spotLightShader = nullptr;

	// Terrain
	Shader* concreteShader = nullptr;

	// Models
	Shader* environmentShader = nullptr; // Fills G-buffers
	Shader* animationShader = nullptr;

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

	Mesh* quad;

private:
	float currentTime;
};

