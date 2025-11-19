#include "Animation.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "nclgl/AssimpNCLHelpers.h"

Animation::Animation(const std::string& animPath, Model* model) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(animPath, aiProcess_Triangulate); // may still include mesh data (it will be ignored)
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "ERROR: ASSIMP: " << importer.GetErrorString() << '\n';
		return;
	}

	aiAnimation* animation = scene->mAnimations[0];
	duration = animation->mDuration;
	ticksPerSec = animation->mTicksPerSecond;
	ReadHierarchyData(*rootNode, scene->mRootNode);
	ReadBones(animation, *model);
}

Bone* Animation::FindBone(const std::string& name) {
	auto iter = std::ranges::find_if(bones, [&](const Bone& bone)->bool { return bone.GetBoneName() == name; });

	if (iter == bones.end()) return nullptr;
	return &(*iter);
}

void Animation::ReadBones(const aiAnimation* anim, Model& model) {
	int size = anim->mNumChannels;

	auto& modelBoneMap = model.GetBoneInfoMap();
	int& numBones = model.GetBoneCount();

	for (int i{}; i < size; ++i) {
		auto channel = anim->mChannels[i]; // each channel represents the bones engaged in an animation and keyframes
		std::string boneName = channel->mNodeName.data;

		if (!modelBoneMap.contains(boneName)) 
			modelBoneMap[boneName].id = numBones++;
		
		bones.emplace_back(boneName, modelBoneMap[boneName].id, channel); // constructs in-place
	}

	boneInfoMap = modelBoneMap;
}

void Animation::ReadHierarchyData(SceneNode& dest, const aiNode* src) {
	assert(src);

	dest.SetName(src->mName.data);
	dest.SetTransform(AssimpNCLHelpers::GetNCLMatrix(src->mTransformation));
	for (int i{}; i < src->mNumChildren; ++i) {
		SceneNode newData;
		ReadHierarchyData(newData, src->mChildren[i]);
		dest.AddChild(&newData);
	}
}
