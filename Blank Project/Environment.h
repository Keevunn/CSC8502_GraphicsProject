#pragma once
#include "Model.h"

#include "nclgl/Light.h"

class Environment : public Model {
public:
	Environment(const std::string& path);

	std::vector<Light> GetStreetLights() { return streetBulbs; }

private:
	void ProcessNode(aiNode* node, const aiScene* scene, SceneNode* parent, Matrix4 parentTransform = Matrix4()) override;
	void LoadMaterials(const aiScene* scene) override;

	std::vector<MaterialTextures> materials;

	std::vector<Light> streetBulbs;
};

