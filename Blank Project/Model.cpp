#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

Model::Model(const std::string& path) {
	LoadModel(path);
}

void Model::Draw(const OGLRenderer& r) {
	for (Mesh* m : meshes) {
		m->Draw();
	}
}

void Model::LoadModel(const std::string& path) {
	Assimp::Importer importer;

	// aiProcess_Triangulate: converts non-triangle faces to triangles
	// aiProcess_GenSmoothNormals: Creates normals if missing
	// aiProcess_CalcTangentSpace: Calculates tangents and bitangents
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "ERROR: ASSIMP: " << importer.GetErrorString() << std::endl;
		return;
	}

	dir = path.substr(0, path.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene) {
	// Process current node's meshes
	for (int i{}; i < node->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(Mesh::LoadFromAssimpMesh(mesh, scene));
		ProcessMaterials(mesh, scene);
	}
	
	// Process children nodes
	for (int i{}; i < node->mNumChildren; ++i)
		ProcessNode(node->mChildren[i], scene);
}

void Model::ProcessMaterials(aiMesh* aiMesh, const aiScene* scene) {
	// Materials
	if (aiMesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
		vector<GLuint> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		diffTex.insert(diffTex.end(), diffuseMaps.begin(), diffuseMaps.end());

		vector<GLuint> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		specularTex.insert(specularTex.end(), specularMaps.begin(), specularMaps.end());
	}
}

vector<GLuint> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
	vector<GLuint> matTextures;
	for (unsigned int i{}; i < mat->GetTextureCount(type); ++i) {
		aiString fileName;
		mat->GetTexture(type, i, &fileName);
		std::string path = dir + "/" + fileName.C_Str();
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
		matTextures.push_back(texID);
	}
	return matTextures;
}

