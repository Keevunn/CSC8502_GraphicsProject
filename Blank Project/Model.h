#pragma once
#include <unordered_map>

#include <assimp/scene.h>
#include "nclgl/OGLRenderer.h"
#include "nclgl/SceneNode.h"
#include "nclgl/Mesh.h"

class Model : public SceneNode {
public:
	Model(const std::string& path);

	std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return boneInfoMap; }
	int& GetBoneCount() { return boneCounter; }

protected:
	void LoadModel(const std::string& path);
	virtual void ProcessNode(aiNode* node, const aiScene* scene, SceneNode* parent);
	virtual void LoadMaterials(const aiScene* scene);

	std::vector<GLuint> textures;
	std::string dir;

	std::unordered_map<std::string, BoneInfo> boneInfoMap{};
	int boneCounter = 0;
};

