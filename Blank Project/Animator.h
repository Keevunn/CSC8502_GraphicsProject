#pragma once
#include <vector>

#include "nclgl/Matrix4.h"

class SceneNode;
class Animation;

class Animator {
public:
	Animator(Animation* anim);
	~Animator();

	void UpdateAnimation(float dt);
	void PlayAnimation(Animation* anim);

	void CalculateBoneTransform(const SceneNode* node, Matrix4 parentTransform);

	std::vector<Matrix4>& GetFinalBoneMatrices() { return finalBoneMatrices; }
	float GetCurrentTime() const { return currentTime; }

private:
	std::vector<Matrix4> finalBoneMatrices;
	Animation* currentAnim = nullptr; // Might change to a vector of animations later
	float currentTime;
	float deltaTime;

	Matrix4 identity;
};

