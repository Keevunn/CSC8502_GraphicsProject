#pragma once
#include <vector>

#include "nclgl/Matrix4.h"
#include "nclgl/Quaternion.h"
#include "nclgl/Vector3.h"

class aiNodeAnim;

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
	Bone(const std::string& name, int ID, const aiNodeAnim* channel);

	void Update(float dt);

	int GetPositionIndex(float dt);
	int GetRotationIndex(float dt);
	int GetScaleIndex(float dt);
	
	Matrix4 GetLocalTransform() { return localTransform; }
	std::string GetBoneName() const { return name; }
	int GetBoneID() { return ID; }

private:
	float GetScaleFactor(float prev, float next, float dt);

	Matrix4 InterpolatePosition(float dt);
	Matrix4 InterpolateRotation(float dt);
	Matrix4 InterpolateScale(float dt);

	std::vector<KeyPosition> positions;
	std::vector<KeyRotation> rotations;
	std::vector<KeyScale> scales;
	int numPositions{};
	int numRotations{};
	int numScalings{};

	std::string name;
	int ID{};
	Matrix4 localTransform;
};

