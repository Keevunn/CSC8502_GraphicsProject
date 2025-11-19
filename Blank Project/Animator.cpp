#include "Animator.h"

#include "Animation.h"
#include "Bone.h"

Animator::Animator(Animation* anim) : currentAnim(anim), currentTime(0), deltaTime(0) {
	identity.ToIdentity();
	finalBoneMatrices.resize(128, identity);
}

void Animator::UpdateAnimation(float dt) {
	deltaTime = dt;
	if (currentAnim) {
		currentTime += currentAnim->GetTicksPerSec() * dt;
		currentTime = fmod(currentTime, currentAnim->GetDuration());
		CalculateBoneTransform(&currentAnim->GetRootNode(), identity);
	}
}

void Animator::PlayAnimation(Animation* anim) {
	currentAnim = anim;
	currentTime = 0;
}

void Animator::CalculateBoneTransform(const SceneNode* node, Matrix4 parentTransform) {
	std::string nodeName = node->GetName();
	Matrix4 nodeTransform = node->GetTransform();

	if (Bone* bone = currentAnim->FindBone(nodeName)) {
		bone->Update(currentTime);
		nodeTransform = bone->GetLocalTransform();
	}
	Matrix4 globalTransformation = parentTransform * nodeTransform;

	auto boneInfoMap = currentAnim->GetBoneInfoMap();
	if (boneInfoMap.contains(nodeName)) {
		int index = boneInfoMap[nodeName].id;
		finalBoneMatrices[index] = globalTransformation * boneInfoMap[nodeName].invBindPose;
	}

	for (auto child = node->GetChildIteratorStart(); child < node->GetChildIteratorEnd(); ++child)
		CalculateBoneTransform(*child, globalTransformation);
}
