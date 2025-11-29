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
	Environment::LoadScene(path, "Robot_Root", flags);
}

void RobotModel::Update(float dt) {
	Environment::Update(dt);

	animator->UpdateAnimation(dt);
}

void RobotModel::SetAnimator(Animation* anim) {
	animator = new Animator(anim);

	// Start animation at random time
	float randomTime = (std::rand() % 100) / 100.0f;
	animator->UpdateAnimation(randomTime);
}

void RobotModel::LoadMaterials(const aiScene* scene) {
	std::string texDir = TEXTUREDIR"Robot/";
	MaterialTextures mat;
	unsigned int flags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS;

	mat.diffuseID = LoadTexture("Robot_Base_color 5.png");
	mat.bumpID = LoadTexture("Robot_Normal_OpenGL_fixed.png");
	mat.alphaID = LoadTexture("Robot_Opacity.png");
	mat.roughnessID = LoadTexture("Robot_Roughness.png");
	mat.metallicID = LoadTexture("Robot_Metallic.png");
	mat.emissiveID = LoadTexture("Robot_Emissive.png");

	materials.push_back(mat);
}

Mesh* RobotModel::LoadMesh(const aiMesh* aiMesh, const aiScene* scene) {
	return Mesh::LoadFromAssimpMesh(aiMesh, scene, boneInfoMap, boneCounter);
}
