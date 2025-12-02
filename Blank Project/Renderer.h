#pragma once
#include "AnimatedModelPool.h"
#include "LampPost.h"
#include "ModelPool.h"
#include "RobotModel.h"
#include "nclgl/OGLRenderer.h"

class Scene;
class NatureScene;
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
	//void DrawPointLights(std::vector<SpotLight*> lights);

	// Factory Scene
	void DrawConcreteFloor(HeightMap* floor, std::unordered_map<std::string, GLuint> floorTextures);
	void DrawFactory(Factory* factory);
	void DrawLampPosts(const ModelPool<LampPost>* lampPosts);
	void DrawRobots(const AnimatedModelPool<RobotModel>* robots);
	void DrawKittens(const AnimatedModelPool<KittenModel>* kittens);
	void DrawKittens(const vector<KittenModel*>& kittens);

	// Nature Scene
	void DrawTerrain(HeightMap* terrain, GLuint blendMapID, std::unordered_map<std::string, GLuint> baseTextures, std::unordered_map<std::string, GLuint> sandTextures,
					std::unordered_map<std::string, GLuint> grassTextures, std::unordered_map<std::string, GLuint> rockTextures);
	void DrawWater(GLuint waterTex, GLuint skybox, HeightMap* waterMap);

	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

	std::shared_ptr<KittenModel> GetGlobalKittenModel() const { return globalKittenModel; }


protected:
	void GenerateScreenTexture(GLuint& into, bool depth = false); // Makes a new texture
	void FillBuffers(); // G-Buffer fill render pass 
	void DrawLights(); // Lighting render pass
	void CombineBuffers(); // Combination render pass

	void DrawNode(SceneNode* n);

	void DrawSun();

	void BindLightUniforms(GLuint programLocation, Matrix4 invViewProj, float* camPos) const;

	FactoryScene* factoryScene;
	NatureScene* natureScene;
	Scene* currentScene;

	// Skybox
	Shader* skyboxShader = nullptr;

	// Light
	Shader* sunShader = nullptr;
	Shader* pointLightShader = nullptr; 
	Shader* spotLightShader = nullptr;

	// Terrain
	Shader* concreteShader = nullptr;
	Shader* natureShader = nullptr;
	Shader* waterShader = nullptr;

	// Models
	Shader* environmentShader = nullptr; // Fills G-buffers
	Shader* animationShader = nullptr;

	std::shared_ptr<KittenModel> globalKittenModel;

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

