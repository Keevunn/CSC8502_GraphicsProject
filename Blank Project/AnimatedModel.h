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

	std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return boneInfoMap; }
	int& GetBoneCount() { return boneCounter; }

	Vector3 GetMovementVelocity() const { return velocity; }
	void SetMovementVelocity(const Vector3& val) { velocity = val; }

	void Move(const Vector3& pos);

protected:
	void LoadScene(const std::string& path, const std::string pName, const int postProcessingFlags) override;
	Mesh* LoadMesh(const aiMesh* aiMesh, const aiScene* scene) override;
	virtual void LoadAnimation(const aiScene* scene);

	virtual aiNode* FindSkeletonRoot(const aiScene* scene) { return nullptr; }
	/*virtual void FindAnimation(const aiScene* scene);*/

	std::unordered_map<std::string, BoneInfo> boneInfoMap{};
	int boneCounter = 0;

	std::shared_ptr<Animation> animation;
	Animator* animator;

	Vector3 velocity;
};

