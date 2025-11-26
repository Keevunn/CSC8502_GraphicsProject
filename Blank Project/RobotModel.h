#pragma once
#include <unordered_map>

#include "Environment.h"

class RobotModel : public Environment {
public:
	RobotModel(const std::string& path);

	std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return boneInfoMap; }
	int& GetBoneCount() { return boneCounter; }

protected:
	void LoadScene(const std::string& path) override;
	void LoadMaterials();
	Mesh* LoadMesh(const aiMesh* aiMesh, const aiScene* scene) override;
	//void LoadMaterials(const aiScene* scene) override;

	
	std::string dir;

	std::unordered_map<std::string, BoneInfo> boneInfoMap{};
	int boneCounter = 0;
};

