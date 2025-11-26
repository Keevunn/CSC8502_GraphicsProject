#pragma once
#include "nclgl/SceneNode.h"

class Environment : public SceneNode {
public:
	Environment() = default;
	Environment(const std::string& path);

	bool HasMaterials() const { return hasMaterials; }

protected:
	virtual void LoadScene(const std::string& path);

	virtual void ProcessNode(aiNode* node, const aiScene* scene, SceneNode* parent);
	virtual Mesh* LoadMesh(const aiMesh* aiMesh, const aiScene* scene);
	virtual void LoadMaterials(const aiScene* scene, std::string texDir);

	std::vector<MaterialTextures> materials;
	bool hasMaterials = false;
};

