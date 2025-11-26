#include "Environment.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "nclgl/AssimpNCLHelpers.h"

// "Lamp Post" (https://skfb.ly/oEOxW) by Rares Orza - skypro86 is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
// "Generic Factory with smoke towers" (https://skfb.ly/onQpR) by assetfactory

Environment::Environment(const std::string& path) {
	LoadScene(path);
}

void Environment::LoadScene(const std::string& path) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GlobalScale | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "ERROR: ASSIMP: " << importer.GetErrorString() << "\n";
		return;
	}

	name = "Environment_Root";

	Environment::LoadMaterials(scene, TEXTUREDIR"/Factory/");
	Environment::ProcessNode(scene->mRootNode, scene, this);
}

void Environment::ProcessNode(aiNode* node, const aiScene* scene, SceneNode* parent) {
	SceneNode* newNode = new SceneNode();
	std::string nodeName = node->mName.C_Str();
	newNode->SetName(nodeName);
	newNode->SetTransform(AssimpNCLHelpers::GetNCLMatrix(node->mTransformation));
	parent->AddChild(newNode);

	int meshChildCount = 0;

	// Process current node's mesh(es)
	for (int i{}; i < node->mNumMeshes; ++i) {
		aiMesh* aiMesh = scene->mMeshes[node->mMeshes[i]];
		Mesh* mesh = LoadMesh(aiMesh, scene);

		SceneNode* currentMeshNode;
		if (i == 0) {
			currentMeshNode = newNode;
			newNode->SetMesh(mesh);
		}
		else { // if this node has multiple meshes, add as child to node
			currentMeshNode = new SceneNode(mesh);
			currentMeshNode->SetName(nodeName + "_Child" + std::to_string(meshChildCount++));
			newNode->AddChild(currentMeshNode);
		}

		//Assign Textures
		// Assumes every mesh only has one material
		int matIndex = aiMesh->mMaterialIndex;
		if (matIndex >= 0 && matIndex < materials.size()) {
			const MaterialTextures mat = materials[matIndex];
			currentMeshNode->SetMaterial(mat);
		}

	}

	// Process children nodes
	for (int i{}; i < node->mNumChildren; ++i)
		ProcessNode(node->mChildren[i], scene, newNode);
}

Mesh* Environment::LoadMesh(const aiMesh* aiMesh, const aiScene* scene) {
	return Mesh::LoadFromAssimpMesh(aiMesh, scene);
}

void Environment::LoadMaterials(const aiScene* scene, const std::string texDir) {
	materials.resize(scene->mNumMaterials);
	if (scene->mNumMaterials > 0) hasMaterials = true;

	for (unsigned int i{}; i < scene->mNumMaterials; ++i) {
		aiMaterial* mat = scene->mMaterials[i];
		aiString fileName;
		GLuint texID;
		unsigned int flags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS;

		if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &fileName) == AI_SUCCESS) {
			std::string path = texDir + fileName.C_Str();
			texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, flags);
			if (!texID) std::cout << "Failed texture: " << fileName.C_Str() << "\n";
			materials[i].diffuseID = texID;
		}

		if (mat->GetTexture(aiTextureType_NORMALS, 0, &fileName) == AI_SUCCESS || 
			mat->GetTexture(aiTextureType_HEIGHT, 0, &fileName) == AI_SUCCESS) {
			std::string path = texDir + fileName.C_Str();
			texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, flags);
			if (!texID) std::cout << "Failed texture: " << fileName.C_Str() << "\n";
			materials[i].bumpID = texID;
		}

		if (mat->GetTexture(aiTextureType_SPECULAR, 0, &fileName) == AI_SUCCESS ||
			mat->GetTexture(aiTextureType_SHININESS, 0, &fileName) == AI_SUCCESS ||
			mat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &fileName) == AI_SUCCESS) {
			std::string path = texDir + fileName.C_Str();
			texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, flags);
			if (!texID) std::cout << "Failed texture: " << fileName.C_Str() << "\n";
			materials[i].specularID = texID;
		}

		if (mat->GetTexture(aiTextureType_REFLECTION, 0, &fileName) == AI_SUCCESS) {
			std::string path = texDir + fileName.C_Str();
			texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, flags);
			if (!texID) std::cout << "Failed texture: " << fileName.C_Str() << "\n";
			materials[i].reflectionID = texID;
		}

		if (mat->GetTexture(aiTextureType_OPACITY, 0, &fileName) == AI_SUCCESS) {
			std::string path = texDir + fileName.C_Str();
			texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, flags);
			if (!texID) std::cout << "Failed texture: " << fileName.C_Str() << "\n";
			materials[i].alphaID = texID;
		}

		if (mat->GetTexture(aiTextureType_METALNESS, 0, &fileName) == AI_SUCCESS) {
			std::string path = texDir + fileName.C_Str();
			texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, flags);
			if (!texID) std::cout << "Failed texture: " << fileName.C_Str() << "\n";
			materials[i].metallicID = texID;
		}

		if (mat->GetTexture(aiTextureType_EMISSIVE, 0, &fileName) == AI_SUCCESS || 
			mat->GetTexture(aiTextureType_EMISSION_COLOR, 0, &fileName) == AI_SUCCESS) {
			std::string path = texDir + fileName.C_Str();
			texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, flags);
			if (!texID) std::cout << "Failed texture: " << fileName.C_Str() << "\n";
			materials[i].emissiveID = texID;
		}
	}
}

