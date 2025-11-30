#include "Animator.h"

#include <utility>

#include "nclgl/SceneNode.h"
#include "Animation.h"

Animator::Animator(const std::shared_ptr<Animation>& anim) : currentAnim(anim), currentTime(0), deltaTime(0) {
	finalBoneMatrices.resize(128, Matrix4());
}

void Animator::UpdateAnimation(float dt) {
	deltaTime = dt;
	if (currentAnim) {
		currentTime += currentAnim->GetTicksPerSec() * dt;
		currentTime = fmod(currentTime, currentAnim->GetDuration());
		CalculateBoneTransform(&currentAnim->GetRootNode(), Matrix4());
	}
}

void Animator::PlayAnimation(const std::shared_ptr<Animation>& anim) {
	currentAnim = anim;
	currentTime = 0;
}

void Animator::CalculateBoneTransform(const SceneNode* node, Matrix4 parentTransform) {
	const auto& boneInfoMap = currentAnim->GetBoneInfoMap();
	std::string nodeName = node->GetName();
	Matrix4 nodeTransform = node->GetTransform();

	if (Bone* bone = currentAnim->FindBone(nodeName)) {
		bone->Update(currentTime);
		nodeTransform = bone->GetLocalTransform();
	}
	Matrix4 globalTransformation = parentTransform * nodeTransform;

	if (boneInfoMap.contains(nodeName)) {
		int index = boneInfoMap.at(nodeName).id;
		finalBoneMatrices[index] = currentAnim->GetGlobalInverseTransform() * globalTransformation * boneInfoMap.at(nodeName).invBindPose;
	}

	for (auto child = node->GetChildIteratorStart(); child < node->GetChildIteratorEnd(); ++child)
		CalculateBoneTransform(*child, globalTransformation);
}
