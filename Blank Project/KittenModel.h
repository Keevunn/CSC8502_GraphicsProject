#pragma once
#include "AnimatedModel.h"

class Animation;

// "Kitten Monk" (https://skfb.ly/oNVLA) by SmugglersStudio

class KittenModel : public AnimatedModel {
public:
	KittenModel(const std::string& path);

protected:
	void LoadMaterials(const aiScene* scene) override;
	aiNode* FindSkeletonRoot(const aiScene* scene) override;

};

