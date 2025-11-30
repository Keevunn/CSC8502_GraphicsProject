#include "RobotModel.h"

#include <assimp/postprocess.h>

#include "Animator.h"

RobotModel::RobotModel(const std::string& path)  {
	auto flags = aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType |
		aiProcess_JoinIdenticalVertices |
		aiProcess_LimitBoneWeights |
		aiProcess_PopulateArmatureData;
	texDir = TEXTUREDIR"Robot/";
	velocity = Vector3(0, 0, 3);
	modelScale = Vector3(1 / 400.0f);
	AnimatedModel::LoadScene(path, "Robot_Root", flags);
}

void RobotModel::LoadMaterials(const aiScene* scene) {
	MaterialTextures mat;

	mat.diffuseID = LoadTexture("Robot_Base_color 5.png");
	mat.bumpID = LoadTexture("Robot_Normal_OpenGL_fixed.png");
	mat.alphaID = LoadTexture("Robot_Opacity.png");
	mat.roughnessID = LoadTexture("Robot_Roughness.png");
	mat.metallicID = LoadTexture("Robot_Metallic.png");
	mat.emissiveID = LoadTexture("Robot_Emissive.png");

	materials.push_back(mat);
}

aiNode* RobotModel::FindSkeletonRoot(const aiScene* scene) {
	aiNode* skeletonRoot = scene->mRootNode->FindNode("mixamorig_Hips"); // May change between animation files (BE CAREFUL)
	if (!skeletonRoot) skeletonRoot = scene->mRootNode;

	return skeletonRoot;
}

