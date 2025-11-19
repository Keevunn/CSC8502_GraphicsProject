#pragma once
#include <unordered_map>

#include <assimp/scene.h>
#include "nclgl/OGLRenderer.h"
#include "nclgl/SceneNode.h"
#include "nclgl/Mesh.h"

class Model : public SceneNode {
public:
	Model(const std::string& path);

	std::vector<GLuint> GetDiffTex() { return diffTex; }
	std::vector<GLuint> GetSpecularTex() { return specularTex; }

	std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return boneInfoMap; }
	int& GetBoneCount() { return boneCounter; }

private:
	void LoadModel(const std::string& path);
	void ProcessNode(aiNode* node, const aiScene* scene, SceneNode* parent);
	void ProcessMaterials(aiMesh* aiMesh, const aiScene* scene);
	void LoadMaterials(const aiScene* scene);
	vector<GLuint> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	std::vector<GLuint> textures;
	vector<GLuint> diffTex;
	vector<GLuint> specularTex;
	std::string dir;

	std::unordered_map<std::string, BoneInfo> boneInfoMap{};
	int boneCounter = 0;
};

