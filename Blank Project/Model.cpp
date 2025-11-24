#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "nclgl/AssimpNCLHelpers.h"

Model::Model(const std::string& path) {
	LoadModel(path);
}

void Model::LoadModel(const std::string& path) {
	Assimp::Importer importer;
	//importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	// aiProcess_Triangulate: converts non-triangle faces to triangles
	// aiProcess_GenSmoothNormals: Creates normals if missing
	// aiProcess_CalcTangentSpace: Calculates tangents and bitangents
	const aiScene* scene = importer.ReadFile(path, 
		aiProcess_Triangulate |
		aiProcess_FlipUVs | 
		aiProcess_GenSmoothNormals | 
		aiProcess_CalcTangentSpace | 
		aiProcess_SortByPType | 
		aiProcess_JoinIdenticalVertices | 
		aiProcess_LimitBoneWeights |
		aiProcess_PopulateArmatureData);
	
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "ERROR: ASSIMP: " << importer.GetErrorString() << std::endl;
		return;
	}
	name = "Model_Root";
	dir = path.substr(0, path.find_last_of('/'));
	LoadMaterials(scene);

	ProcessNode(scene->mRootNode, scene, this);
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
		Mesh* mesh = Mesh::LoadFromAssimpMesh(aiMesh, scene, boneInfoMap, boneCounter);

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

void Model::LoadMaterials(const aiScene* scene) {
	textures.resize(scene->mNumMaterials, 0);

	for (unsigned int i{}; i < scene->mNumMaterials; ++i) {
		aiMaterial* mat = scene->mMaterials[i];

		if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString fileName;
			mat->GetTexture(aiTextureType_DIFFUSE, 0, &fileName);
			std::string path = dir + "/" + fileName.C_Str();
			GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
			textures[i] = texID;
		}
	}
}