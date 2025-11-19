#pragma once
#include <vector>

#include "nclgl/Matrix4.h"
#include "nclgl/Quaternion.h"
#include "nclgl/Vector3.h"

struct aiNodeAnim;

struct KeyPosition {
	Vector3	position;
	float timeStamp;
};

struct KeyRotation {
	Quaternion orientation;
	float timeStamp;
};

struct KeyScale {
	Vector3 scale;
	float timeStamp;
};

class Bone {
public:
	Bone(std::string name, int ID, const aiNodeAnim* channel);

	void Update(float anim_dt);

	int GetPositionIndex(float anim_dt);
	int GetRotationIndex(float anim_dt);
	int GetScaleIndex(float anim_dt);
	
	Matrix4 GetLocalTransform() { return localTransform; }
	std::string GetBoneName() const { return name; }
	int GetBoneID() { return ID; }

private:
	float GetScaleFactor(float prev, float next, float anim_dt);

	Matrix4 InterpolatePosition(float anim_dt);
	Matrix4 InterpolateRotation(float anim_dt);
	Matrix4 InterpolateScale(float anim_dt);

	std::vector<KeyPosition> positions;
	std::vector<KeyRotation> rotations;
	std::vector<KeyScale> scales;
	int numPositions{};
	int numRotations{};
	int numScales{};

	std::string name;
	int ID{};
	Matrix4 localTransform;
};

