#pragma once
#include <string>
#include <unordered_map>

#include "AnimatedModelPool.h"
#include "ModelPool.h"
#include "Scene.h"

class Factory;
class RobotModel;
class KittenModel;
class LampPost;

class FactoryScene : public Scene {
public:
	FactoryScene() = default;
	FactoryScene(float width, float height) { FactoryScene::Initialise(width, height); }
	~FactoryScene() override;

	void Initialise(float width, float height) override;
	void Update(float dt) override;

	void RenderGeometry(Renderer& renderer) override;
	void RenderLights(Renderer& renderer) override;

private:
	// Terrain
	HeightMap* concreteMap = nullptr;
	std::unordered_map<std::string, GLuint> concreteTextures; // texture type, texture id

	// Environment objects
	Factory* factory = nullptr;
	ModelPool<LampPost>* lampPosts;

	AnimatedModelPool<RobotModel>* robots;
	AnimatedModelPool<KittenModel>* kittens;

};

