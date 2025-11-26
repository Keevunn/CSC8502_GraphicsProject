#pragma once
#include "nclgl/SceneNode.h"

class Environment : public SceneNode {
public:
	Environment() = default;

	bool HasMaterials() const { return hasMaterials; }

protected:
	virtual void LoadScene(const std::string& path, const std::string pName, const int postProcessingFlags);

	virtual void ProcessNode(aiNode* node, const aiScene* scene, SceneNode* pParent);

	virtual Mesh* LoadMesh(const aiMesh* aiMesh, const aiScene* scene);
	virtual void LoadMaterials(const aiScene* scene);
	GLuint LoadTexture(const std::string filename, const bool useRGB = false) const;

	std::string texDir;

	bool hasMaterials = false;
	std::vector<MaterialTextures> materials;
};

