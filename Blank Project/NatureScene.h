#pragma once
#include "AnimatedModelPool.h"
#include "Scene.h"
class KittenModel;

class NatureScene : public Scene {
public:
	NatureScene() = default;
	NatureScene(Renderer& renderer) { NatureScene::Initialise(renderer); }
	~NatureScene();

	void Initialise(Renderer& renderer) override;
	void Update(float dt) override;

	void RenderGeometry(Renderer& renderer) override;
	void RenderLights(Renderer& renderer) override {}

private:
	float GetTerrainHeight(float x, float z, int mapWidth, int mapHeight);

	HeightMap* terrain = nullptr;
	unsigned char* terrainData = nullptr;
	GLuint terrainBlendMap;
	std::unordered_map<std::string, GLuint> baseTextures;
	std::unordered_map<std::string, GLuint> sandTextures;
	std::unordered_map<std::string, GLuint> grassTextures;
	std::unordered_map<std::string, GLuint> rockyTextures;

	HeightMap* waterHeightMap = nullptr;
	GLuint waterMask;

	std::vector<KittenModel*> kittens;
};

