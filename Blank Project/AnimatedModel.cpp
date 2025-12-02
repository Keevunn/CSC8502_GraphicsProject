#include "AnimatedModel.h"

#include <cassert>
#include <assimp/Importer.hpp>

#include "Animation.h"
#include "nclgl/AssimpNCLHelpers.h"

AnimatedModel::AnimatedModel(const AnimatedModel& other) : Environment(other) {
	boneInfoMap = other.boneInfoMap;
	boneCounter = other.boneCounter;
	animationMap = other.animationMap;
	animator = other.animator ? new Animator(*other.animator) : nullptr;
	velocity = other.velocity;
}

AnimatedModel::~AnimatedModel() {
	delete animator;
}

void AnimatedModel::Update(float dt) {
	Environment::Update(dt);
	animator->UpdateAnimation(dt);

	if (velocity != Vector3(0))
		transform = transform * Matrix4::Translation(velocity * dt);
}

void AnimatedModel::SetAnimator(const std::shared_ptr<Animation>& anim) {
	if (!animator) {
		animator = new Animator(anim);
		// Start animation at random time
		float randomTime = (std::rand() % 100) / 100.0f;
		animator->PlayAnimation(anim, randomTime);
	}
	
}

void AnimatedModel::Move(const Vector3& pos) {
	transform = transform * Matrix4::Translation(pos);
}

void AnimatedModel::LoadScene(const std::string& path, const std::string pName, const int postProcessingFlags, bool sanitiseInput, const std::string& animName) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, postProcessingFlags);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "ERROR: ASSIMP: " << importer.GetErrorString() << "\n";
		return;
	}

	name = pName;

	LoadMaterials(scene);
	ProcessNode(scene->mRootNode, scene, this);
	LoadAnimation(scene, animName, sanitiseInput);
}

Mesh* AnimatedModel::LoadMesh(const aiMesh* aiMesh, const aiScene* scene) {
	return Mesh::LoadFromAssimpMesh(aiMesh, scene, boneInfoMap, boneCounter);
}

// Assumes each scene has a single animation

void AnimatedModel::LoadAnimation(const aiScene* scene, const std::string& animName, bool sanitiseInput) {
	assert(scene->mNumAnimations != 0);
	const aiAnimation* anim = scene->mAnimations[0];
	const aiNode* skeltonRoot = FindSkeletonRoot(scene);
	const Matrix4 globalInverseTransform = AssimpNCLHelpers::GetNCLMatrix(scene->mRootNode->mTransformation).Inverse();

	auto animation = std::make_shared<Animation>(anim, skeltonRoot, boneInfoMap, globalInverseTransform, sanitiseInput);
	animationMap[animName] = std::move(animation);
	SetAnimator(animationMap[animName]);
	
}


