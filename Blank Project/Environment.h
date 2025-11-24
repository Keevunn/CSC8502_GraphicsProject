#pragma once
#include "Model.h"

class Environment : public Model {
public:
	Environment(const std::string& path);

private:
	void ProcessNode(aiNode* node, const aiScene* scene, SceneNode* parent) override;
	void LoadMaterials(const aiScene* scene) override;

	std::vector<MaterialTextures> materials;
};

