#pragma once
#include "AnimatedModel.h"

class Animation;

// "Kitten Monk" (https://skfb.ly/oNVLA) by SmugglersStudio

class KittenModel : public AnimatedModel {
public:
	KittenModel(const std::string& path, const Vector3& velocity = Vector3(0), const std::string& animName = "");

	void LoadAnimationFromFile(const std::string& path, const std::string& animName);

protected:
	void LoadMaterials(const aiScene* scene) override;
	aiNode* FindSkeletonRoot(const aiScene* scene) override;

};

