#include "Animation.h"

#include <algorithm>
#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "nclgl/AssimpNCLHelpers.h"
#include "nclgl/SceneNode.h"

Animation::Animation(const std::string& animPath, const std::unordered_map<std::string, BoneInfo>& modelBoneMap) { // Check if being used
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	const aiScene* scene = importer.ReadFile(animPath, aiProcess_Triangulate); // may still include mesh data (it will be ignored)
	assert(scene && scene->mRootNode);

	aiAnimation* animation = scene->mAnimations[0];
	duration = animation->mDuration;
	ticksPerSec = animation->mTicksPerSecond;

	const aiNode* skeletonRoot = scene->mRootNode->FindNode("mixamorig_Hips"); // May change between animation files (BE CAREFUL)
	if (!skeletonRoot) skeletonRoot = scene->mRootNode;

	globalInverseTransform = (AssimpNCLHelpers::GetNCLMatrix(scene->mRootNode->mTransformation)).Inverse();

	rootNode = new SceneNode();
	ReadHierarchyData(*rootNode, skeletonRoot, true);
	ReadBones(animation, modelBoneMap, true);
}

Animation::Animation(const aiAnimation* animation, const aiNode* skeletonRoot, const std::unordered_map<std::string, BoneInfo>& modelBoneMap, const Matrix4& globalInverseTransform, bool sanitiseInput)
: globalInverseTransform(globalInverseTransform) {
	duration = animation->mDuration;
	ticksPerSec = animation->mTicksPerSecond;

	rootNode = new SceneNode();
	ReadHierarchyData(*rootNode, skeletonRoot, sanitiseInput);
	ReadBones(animation, modelBoneMap, sanitiseInput);
}

Animation::~Animation() {
	delete rootNode;
}

Bone* Animation::FindBone(const std::string& name) {
	auto iter = std::ranges::find_if(bones, [&](const Bone& bone)->bool { return bone.GetBoneName() == name; });

	if (iter == bones.end()) return nullptr;
	return &(*iter);
}

void Animation::ReadBones(const aiAnimation* anim, const std::unordered_map<std::string, BoneInfo>& modelBoneMap, bool sanitiseInput) {
	int size = anim->mNumChannels;

	for (int i{}; i < size; ++i) {
		auto channel = anim->mChannels[i]; // each channel represents the bones engaged in an animation and keyframes
		std::string boneName = channel->mNodeName.data;

		if (sanitiseInput) {
			auto colonPos = boneName.find_first_of(':');
			if (colonPos != std::string::npos)
				boneName.replace(colonPos, 1, "_");
		}

		int boneID = -1;
		if (modelBoneMap.contains(boneName))
			boneID = modelBoneMap.at(boneName).id;

		bones.emplace_back(boneName, boneID, channel); // constructs in-place
	}

	boneInfoMap = modelBoneMap;
}

void Animation::ReadHierarchyData(SceneNode& dest, const aiNode* src, bool sanitiseInput) {
	assert(src);

	std::string nodeName = src->mName.C_Str();
	if (sanitiseInput) {
		auto colonPos = nodeName.find_first_of(':');
		if (colonPos != std::string::npos)
			nodeName.replace(colonPos, 1, "_");
	}
	

	dest.SetName(nodeName);
	dest.SetTransform(AssimpNCLHelpers::GetNCLMatrix(src->mTransformation));

	for (int i{}; i < src->mNumChildren; ++i) {
		SceneNode* newData = new SceneNode();
		ReadHierarchyData(*newData, src->mChildren[i], sanitiseInput);
		dest.AddChild(newData);
	}
}
