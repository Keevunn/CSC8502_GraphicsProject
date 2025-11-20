#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "nclgl/AssimpNCLHelpers.h"

Model::Model(const std::string& path) {
	LoadModel(path);
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
	name = "Model_Root";
	dir = path.substr(0, path.find_last_of('/'));
	LoadMaterials(scene);

	ProcessNode(scene->mRootNode, scene, this);
	boneCounter = boneInfoMap.size();
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, SceneNode* parent) {
	SceneNode* newNode = new SceneNode();
	std::string nodeName = node->mName.C_Str();
	newNode->SetName(nodeName);
	newNode->SetTransform(AssimpNCLHelpers::GetNCLMatrix(node->mTransformation));
	parent->AddChild(newNode);

	int meshChildCount = 0;

	// Process current node's mesh(es)
	for (int i{}; i < node->mNumMeshes; ++i) {
		aiMesh* aiMesh = scene->mMeshes[node->mMeshes[i]];
		Mesh* mesh = Mesh::LoadFromAssimpMesh(aiMesh, scene);
		
		// Update bone information
		boneInfoMap.merge(mesh->GetBoneInfoMap());

		SceneNode* currentMeshNode = nullptr;
		if (i == 0) {
			currentMeshNode = newNode;
			newNode->SetMesh(mesh);
		}
		else { // if this node has multiple meshes, add as child to node
			currentMeshNode = new SceneNode(mesh);
			currentMeshNode->SetName(nodeName + "_Child" + std::to_string(meshChildCount++));
			newNode->AddChild(currentMeshNode);
		}

		//Assign Texture
		int matIndex = aiMesh->mMaterialIndex;
		if (matIndex >= 0 && matIndex < textures.size()) {
			GLuint texID = textures[matIndex];
			if (texID != 0) currentMeshNode->SetTexture(texID);
		}

	}
	
	// Process children nodes
	for (int i{}; i < node->mNumChildren; ++i)
		ProcessNode(node->mChildren[i], scene, newNode);
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

void Model::LoadMaterials(const aiScene* scene) {
	textures.resize(scene->mNumMaterials, 0);

	for (unsigned int i{}; i < scene->mNumMaterials; ++i) {
		aiMaterial* mat = scene->mMaterials[i];

		if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString fileName;
			mat->GetTexture(aiTextureType_DIFFUSE, i, &fileName);
			std::string path = dir + "/" + fileName.C_Str();
			GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
			textures[i] = texID;
		}
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

