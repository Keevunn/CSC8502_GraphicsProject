#pragma once
#include <unordered_map>

#include "AnimatedModel.h"
#include "Animation.h"
#include "Animator.h"
#include "Environment.h"

// "Robot" (https://www.turbosquid.com/3d-models/robot-free-3d-model-2284745) by Roman Yuldashev

class RobotModel : public AnimatedModel {
public:
	RobotModel(const std::string& path);

protected:
	void LoadMaterials(const aiScene* scene) override;
	aiNode* FindSkeletonRoot(const aiScene* scene) override;

};

