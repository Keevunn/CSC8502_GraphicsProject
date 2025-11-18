#pragma once
#include <assimp/scene.h>
#include "nclgl/OGLRenderer.h"
class Mesh;

class Model {
public:
	Model(const std::string& path);
	~Model() = default;

	void Draw(const OGLRenderer& r);

	std::vector<Mesh*> GetMeshes() { return meshes; }
	std::vector<GLuint> GetDiffTex() { return diffTex; }
	std::vector<GLuint> GetSpecularTex() { return specularTex; }

private:
	void LoadModel(const std::string& path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	void ProcessMaterials(aiMesh* aiMesh, const aiScene* scene);
	vector<GLuint> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	std::vector<Mesh*> meshes;
	vector<GLuint> diffTex;
	vector<GLuint> specularTex;
	std::string dir;
};

