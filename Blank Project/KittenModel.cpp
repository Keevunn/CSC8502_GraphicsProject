#include "KittenModel.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Animator.h"

KittenModel::KittenModel(const std::string& path) {
	auto flags = aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType |
		aiProcess_JoinIdenticalVertices |
		aiProcess_LimitBoneWeights |
		aiProcess_PopulateArmatureData;
	texDir = TEXTUREDIR;
	velocity = Vector3(-3, 0, 0);
	AnimatedModel::LoadScene(path, "Kitten_Root", flags);
}

void KittenModel::LoadMaterials(const aiScene* scene) {
	MaterialTextures mat;

	mat.diffuseID = LoadTexture("Kitten.png");

	materials.push_back(mat);
}

aiNode* KittenModel::FindSkeletonRoot(const aiScene* scene) {
	aiNode* skeletonRoot = scene->mRootNode->FindNode("root.x"); // May change between animation files (BE CAREFUL)
	if (!skeletonRoot) skeletonRoot = scene->mRootNode;

	return skeletonRoot;
}
