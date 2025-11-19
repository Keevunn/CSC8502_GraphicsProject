#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include <assimp/anim.h>

#include "Bone.h"
#include "Model.h"

#include "nclgl/Mesh.h"

class Animation {
public:
	Animation() = default;
	Animation(const std::string& animPath, Model* model);
	~Animation() = default;

	Bone* FindBone(const std::string& name);

	float GetTicksPerSec() const { return ticksPerSec; }
	float GetDuration() const { return duration; }
	SceneNode& GetRootNode() const { return *rootNode; }
	std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return boneInfoMap; }


private:
	void ReadBones(const aiAnimation* anim, Model& model);
	void ReadHierarchyData(SceneNode& dest, const aiNode* src);

	float duration;
	float ticksPerSec;
	std::vector<Bone> bones;
	SceneNode* rootNode{};
	std::unordered_map<std::string, BoneInfo> boneInfoMap;
};

