#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include <assimp/anim.h>

#include "Bone.h"

#include "nclgl/Mesh.h"

class SceneNode;

class Animation {
public:
	Animation() = default;
	Animation(const std::string& animPath, const std::unordered_map<std::string, BoneInfo>& modelBoneMap);
	Animation(const aiAnimation* animation, const aiNode* skeletonRoot, const std::unordered_map<std::string, BoneInfo>& modelBoneMap, const Matrix4& globalInverseTransform = Matrix4());
	~Animation();

	Bone* FindBone(const std::string& name);

	float GetTicksPerSec() const { return ticksPerSec; }
	float GetDuration() const { return duration; }

	SceneNode& GetRootNode() const { return *rootNode; }
	Matrix4& GetGlobalInverseTransform() { return globalInverseTransform; }

	std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return boneInfoMap; }

private:
	void ReadBones(const aiAnimation* anim, const std::unordered_map<std::string, BoneInfo>& modelBoneMap);
	void ReadHierarchyData(SceneNode& dest, const aiNode* src);

	float duration;
	float ticksPerSec;

	SceneNode* rootNode = nullptr;
	Matrix4 globalInverseTransform;

	std::vector<Bone> bones;
	std::unordered_map<std::string, BoneInfo> boneInfoMap;
};

