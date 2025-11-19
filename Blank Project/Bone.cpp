#include "Bone.h"

#include <assimp/anim.h>

#include "nclgl/AssimpNCLHelpers.h"
#include "nclgl/Matrix3.h"

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel) : name(name), ID(ID) {
	numPositions = channel->mNumPositionKeys;
	for (int posIndex = 0; posIndex < numPositions; ++ posIndex) {
		KeyPosition data;
		data.position = AssimpNCLHelpers::GetNCLVec(channel->mPositionKeys[posIndex].mValue);
		data.timeStamp = channel->mPositionKeys[posIndex].mTime;
		positions.push_back(data);
	}

	numRotations = channel->mNumRotationKeys;
	for (int rotIndex = 0; rotIndex < numRotations; ++rotIndex) {
		KeyRotation data;
		data.orientation = AssimpNCLHelpers::GetNCLQuat(channel->mRotationKeys[rotIndex].mValue);
		data.timeStamp = channel->mRotationKeys[rotIndex].mTime;
		rotations.push_back(data);
	}

	numScalings = channel->mNumScalingKeys;
	for (int scaleIndex = 0; scaleIndex < numScalings; ++scaleIndex) {
		KeyScale data;
		data.scale = AssimpNCLHelpers::GetNCLVec(channel->mScalingKeys[scaleIndex].mValue);
		data.timeStamp = channel->mScalingKeys[scaleIndex].mTime;
		scales.push_back(data);
	}
}

void Bone::Update(float dt) {
	Matrix4 translation = InterpolatePosition(dt);
	Matrix4 rotation = InterpolateRotation(dt);
	Matrix4 scale = InterpolateScale(dt);
	localTransform = translation * rotation * scale;
}

int Bone::GetPositionIndex(float dt) {
	for (int i{}; i < numPositions - 1; ++i)
		if (dt < positions[i + 1].timeStamp) return i;
}

int Bone::GetRotationIndex(float dt) {
	for (int i{}; i < numRotations - 1; ++i)
		if (dt < rotations[i + 1].timeStamp) return i;
}

int Bone::GetScaleIndex(float dt) {
	for (int i{}; i < numScalings - 1; ++i)
		if (dt < scales[i + 1].timeStamp) return i;
}

float Bone::GetScaleFactor(float prev, float next, float dt) {
	float length = dt - prev;
	float framesDiff = next - prev;
	return length / framesDiff; // normalised value for lerp and slerp 
}

Matrix4 Bone::InterpolatePosition(float dt) {
	if (numPositions == 1) return Matrix4::Translation(positions[0].position);

	int posIndex0 = GetPositionIndex(dt);
	int posIndex1 = posIndex0 + 1;
	float scaleFactor = GetScaleFactor(positions[posIndex0].timeStamp, positions[posIndex1].timeStamp, dt);
	Vector3 finalPos = positions[posIndex0].position * (1 - scaleFactor) + positions[posIndex1].position * scaleFactor;

	return Matrix4::Translation(finalPos);
}

Matrix4 Bone::InterpolateRotation(float dt) {
	if (numRotations == 1) {
		auto rot = rotations[0].orientation;
		rot.Normalise();
		Vector3 pyr = rot.ToEuler();
		Matrix4 pitch = Matrix4::Rotation(pyr.x, Vector3(1, 0, 0));
		Matrix4 yaw = Matrix4::Rotation(pyr.y, Vector3(0, 1, 0));
		Matrix4 roll = Matrix4::Rotation(pyr.z, Vector3(0, 0, 1));
		return pitch * yaw * roll;
	}

	int rotIndex0 = GetRotationIndex(dt);
	int rotIndex1 = rotIndex0 + 1;
	float scaleFactor = GetScaleFactor(rotations[rotIndex0].timeStamp, rotations[rotIndex1].timeStamp, dt);
	Quaternion finalRot = Quaternion::Slerp(rotations[rotIndex0].orientation, rotations[rotIndex1].orientation, scaleFactor);
	finalRot.Normalise();
	
	Vector3 pyr = finalRot.ToEuler();
	Matrix4 pitch = Matrix4::Rotation(pyr.x, Vector3(1, 0, 0));
	Matrix4 yaw = Matrix4::Rotation(pyr.y, Vector3(0, 1, 0));
	Matrix4 roll = Matrix4::Rotation(pyr.z, Vector3(0, 0, 1));
	return pitch * yaw * roll;
}

Matrix4 Bone::InterpolateScale(float dt) {
	if (numScalings == 1) return Matrix4::Scale(scales[0].scale);

	int scaleIndex0 = GetScaleIndex(dt);
	int scaleIndex1 = scaleIndex0 + 1;
	float scaleFactor = GetScaleFactor(scales[scaleIndex0].timeStamp, scales[scaleIndex1].timeStamp, dt);
	Vector3 finalScale = scales[scaleIndex0].scale * (1 - scaleFactor) + scales[scaleIndex1].scale * scaleFactor;

	return Matrix4::Scale(finalScale);
}