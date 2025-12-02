#pragma once
#include <vector>

#include "nclgl/Matrix4.h"

class SceneNode;
class Animation;

class Animator {
public:
	Animator(const std::shared_ptr<Animation>& anim);
	Animator(const Animator& other);

	void UpdateAnimation(float dt);
	void PlayAnimation(const std::shared_ptr<Animation>& anim, float startTime = 0);

	void CalculateBoneTransform(const SceneNode* node, Matrix4 parentTransform);

	std::vector<Matrix4>& GetFinalBoneMatrices() { return finalBoneMatrices; }
	float GetCurrentTime() const { return currentTime; }

private:
	std::vector<Matrix4> finalBoneMatrices;
	std::shared_ptr<Animation> currentAnim = nullptr; // Deleted in AnimatedModel
	float currentTime;
};

