#pragma once
#include <vector>

#include "nclgl/Matrix4.h"

class SceneNode;
class Animation;

class Animator {
public:
	Animator(Animation* anim);
	~Animator() = default;

	void UpdateAnimation(float dt);
	void PlayAnimation(Animation* anim);

	void CalculateBoneTransform(const SceneNode* node, Matrix4 parentTransform);

	std::vector<Matrix4> GetFinalBoneMatrices() { return finalBoneMatrices; }

private:
	std::vector<Matrix4> finalBoneMatrices;
	Animation* currentAnim;
	float currentTime;
	float deltaTime;

	Matrix4 identity;

	int output = 0;
};

