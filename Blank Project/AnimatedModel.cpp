#include "AnimatedModel.h"

#include <cassert>
#include <assimp/Importer.hpp>

#include "Animation.h"
#include "nclgl/AssimpNCLHelpers.h"

AnimatedModel::AnimatedModel(const AnimatedModel& other) : Environment(other) {
	boneInfoMap = other.boneInfoMap;
	boneCounter = other.boneCounter;
	animation = other.animation;
	SetAnimator(animation);
	velocity = other.velocity;
}

AnimatedModel::~AnimatedModel() {
	delete animator;
}

void AnimatedModel::Update(float dt) {
	Environment::Update(dt);
	animator->UpdateAnimation(dt);

	transform = transform * Matrix4::Translation(velocity * dt);
}

void AnimatedModel::SetAnimator(const std::shared_ptr<Animation>& anim) {
	animator = new Animator(anim);

	// Start animation at random time
	float randomTime = (std::rand() % 100) / 100.0f;
	animator->UpdateAnimation(randomTime);
}

void AnimatedModel::Move(const Vector3& pos) {
	transform = transform * Matrix4::Translation(pos);
}

void AnimatedModel::LoadScene(const std::string& path, const std::string pName, const int postProcessingFlags) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, postProcessingFlags);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "ERROR: ASSIMP: " << importer.GetErrorString() << "\n";
		return;
	}

	name = pName;

	LoadMaterials(scene);
	ProcessNode(scene->mRootNode, scene, this);
	LoadAnimation(scene);
}

Mesh* AnimatedModel::LoadMesh(const aiMesh* aiMesh, const aiScene* scene) {
	return Mesh::LoadFromAssimpMesh(aiMesh, scene, boneInfoMap, boneCounter);
}

void AnimatedModel::LoadAnimation(const aiScene* scene) {
	assert(scene->mNumAnimations != 0);
	const aiAnimation* anim = scene->mAnimations[0];
	const aiNode* skeltonRoot = FindSkeletonRoot(scene);
	const Matrix4 globalInverseTransform = AssimpNCLHelpers::GetNCLMatrix(scene->mRootNode->mTransformation).Inverse();

	animation = std::make_shared<Animation>(anim, skeltonRoot, boneInfoMap, globalInverseTransform);
	SetAnimator(animation);
}


