#pragma once
#include "Animator.h"
#include "Environment.h"

class Animation;


class AnimatedModel : public Environment {
public:
	AnimatedModel() = default;
	AnimatedModel(const AnimatedModel& other);
	~AnimatedModel() override;

	void Update(float dt) override;

	vector<Matrix4>& GetFinalBoneMatrices() const { return animator->GetFinalBoneMatrices(); }

	void SetAnimator(const std::shared_ptr<Animation>& anim);
	void PlayAnimation(const std::string& animName) const { animator->PlayAnimation(animationMap.at(animName)); }
	std::unordered_map<std::string, std::shared_ptr<Animation>> GetAnimationMap() const { return animationMap; }

	std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return boneInfoMap; }
	int& GetBoneCount() { return boneCounter; }

	Vector3 GetMovementVelocity() const { return velocity; }
	void SetMovementVelocity(const Vector3& val) { velocity = val; }

	void Move(const Vector3& pos);

protected:
	void LoadScene(const std::string& path, const std::string pName, const int postProcessingFlags, bool sanitiseInput = true, const std::string& animName = "");
	Mesh* LoadMesh(const aiMesh* aiMesh, const aiScene* scene) override;
	void LoadAnimation(const aiScene* scene, const std::string& animName, bool sanitiseInput);

	virtual aiNode* FindSkeletonRoot(const aiScene* scene) { return nullptr; }
	/*virtual void FindAnimation(const aiScene* scene);*/

	std::unordered_map<std::string, BoneInfo> boneInfoMap{};
	int boneCounter = 0;

	std::unordered_map<std::string, std::shared_ptr<Animation>> animationMap{};
	Animator* animator = nullptr;

	Vector3 velocity;
};

