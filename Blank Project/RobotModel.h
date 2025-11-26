#pragma once
#include <unordered_map>

#include "Environment.h"

// "Robot" (https://www.turbosquid.com/3d-models/robot-free-3d-model-2284745) by Roman Yuldashev

class RobotModel : public Environment {
public:
	RobotModel(const std::string& path);

	std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return boneInfoMap; }
	int& GetBoneCount() { return boneCounter; }

protected:
	void LoadMaterials(const aiScene* scene) override;
	Mesh* LoadMesh(const aiMesh* aiMesh, const aiScene* scene) override;

	std::unordered_map<std::string, BoneInfo> boneInfoMap{};
	int boneCounter = 0;
};

