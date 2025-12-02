#include "KittenModel.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

KittenModel::KittenModel(const std::string& path, const Vector3& velocity, const std::string& animName) {
	auto flags = aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType |
		aiProcess_JoinIdenticalVertices |
		aiProcess_LimitBoneWeights |
		aiProcess_PopulateArmatureData;
	texDir = TEXTUREDIR"Kitten/";
	this->velocity = velocity;
	modelScale = Vector3(1 / 9000.0f);
	transform =  Matrix4::Rotation(90, Vector3(1, 0, 0));
	LoadScene(path, "Kitten_Root", flags, false, animName);
}

void KittenModel::LoadAnimationFromFile(const std::string& path, const std::string& animName) {
	auto flags = aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType |
		aiProcess_JoinIdenticalVertices |
		aiProcess_LimitBoneWeights |
		aiProcess_PopulateArmatureData;

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, flags);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "ERROR: ASSIMP: " << importer.GetErrorString() << "\n";
		return;
	}

	LoadAnimation(scene, animName, false);
}

void KittenModel::LoadMaterials(const aiScene* scene) {
	MaterialTextures mat;
	mat.diffuseID = LoadTexture("pink-cat-textures.png");
	materials.push_back(mat);
	
	mat.diffuseID = LoadTexture("DefaultEyes.png");
	materials.push_back(mat);
}

aiNode* KittenModel::FindSkeletonRoot(const aiScene* scene) {
	aiNode* skeletonRoot = scene->mRootNode->FindNode("ArmatureDefaultCat2.002"); // May change between animation files (BE CAREFUL)
	if (!skeletonRoot) skeletonRoot = scene->mRootNode;
	return skeletonRoot;
}
