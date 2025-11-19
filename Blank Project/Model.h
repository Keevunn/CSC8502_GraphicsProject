#pragma once
#include <assimp/scene.h>
#include "nclgl/OGLRenderer.h"
#include "nclgl/SceneNode.h"
class Mesh;

class Model : public SceneNode {
public:
	Model(const std::string& path);

	std::vector<GLuint> GetDiffTex() { return diffTex; }
	std::vector<GLuint> GetSpecularTex() { return specularTex; }

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
};

