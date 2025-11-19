#include "Bone.h"

#include <assimp/anim.h>

#include "nclgl/AssimpNCLHelpers.h"

Bone::Bone(std::string name, const int ID, const aiNodeAnim* channel) : name(std::move(name)), ID(ID) {
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

	numScales = channel->mNumScalingKeys;
	for (int scaleIndex = 0; scaleIndex < numScales; ++scaleIndex) {
		KeyScale data;
		data.scale = AssimpNCLHelpers::GetNCLVec(channel->mScalingKeys[scaleIndex].mValue);
		data.timeStamp = channel->mScalingKeys[scaleIndex].mTime;
		scales.push_back(data);
	}
}

void Bone::Update(const float anim_dt) {
	Matrix4 translation = InterpolatePosition(anim_dt);
	Matrix4 rotation = InterpolateRotation(anim_dt);
	Matrix4 scale = InterpolateScale(anim_dt);
	localTransform = translation * rotation * scale;
}

int Bone::GetPositionIndex(const float anim_dt) {
	for (int i{}; i < numPositions - 1; ++i)
		if (anim_dt < positions[i + 1].timeStamp) return i;
}

int Bone::GetRotationIndex(const float anim_dt) {
	for (int i{}; i < numRotations - 1; ++i)
		if (anim_dt < rotations[i + 1].timeStamp) return i;
}

int Bone::GetScaleIndex(const float anim_dt) {
	for (int i{}; i < numScales - 1; ++i)
		if (anim_dt < scales[i + 1].timeStamp) return i;
}

float Bone::GetScaleFactor(const float prev, const float next, const float anim_dt) {
	float length = anim_dt - prev;
	float totalFrames = next - prev;
	return length / totalFrames; // normalised value for lerp and slerp 
}

Matrix4 Bone::InterpolatePosition(const float anim_dt) {
	if (numPositions == 1) return Matrix4::Translation(positions[0].position);

	int posIndex0 = GetPositionIndex(anim_dt);
	int posIndex1 = posIndex0 + 1;
	float scaleFactor = GetScaleFactor(positions[posIndex0].timeStamp, positions[posIndex1].timeStamp, anim_dt);
	Vector3 finalPos = positions[posIndex0].position * (1 - scaleFactor) + positions[posIndex1].position * scaleFactor;

	return Matrix4::Translation(finalPos);
}

Matrix4 Bone::InterpolateRotation(const float anim_dt) {
	if (numRotations == 1) {
		auto rot = rotations[0].orientation;
		rot.Normalise();
		Vector3 pyr = rot.ToEuler();
		Matrix4 pitch = Matrix4::Rotation(pyr.x, Vector3(1, 0, 0));
		Matrix4 yaw = Matrix4::Rotation(pyr.y, Vector3(0, 1, 0));
		Matrix4 roll = Matrix4::Rotation(pyr.z, Vector3(0, 0, 1));
		return pitch * yaw * roll;
	}

	int rotIndex0 = GetRotationIndex(anim_dt);
	int rotIndex1 = rotIndex0 + 1;
	float scaleFactor = GetScaleFactor(rotations[rotIndex0].timeStamp, rotations[rotIndex1].timeStamp, anim_dt);
	Quaternion finalRot = Quaternion::Slerp(rotations[rotIndex0].orientation, rotations[rotIndex1].orientation, scaleFactor);
	finalRot.Normalise();
	
	Vector3 pyr = finalRot.ToEuler();
	Matrix4 pitch = Matrix4::Rotation(pyr.x, Vector3(1, 0, 0));
	Matrix4 yaw = Matrix4::Rotation(pyr.y, Vector3(0, 1, 0));
	Matrix4 roll = Matrix4::Rotation(pyr.z, Vector3(0, 0, 1));
	return pitch * yaw * roll;
}

Matrix4 Bone::InterpolateScale(const float anim_dt) {
	if (numScales == 1) return Matrix4::Scale(scales[0].scale);

	int scaleIndex0 = GetScaleIndex(anim_dt);
	int scaleIndex1 = scaleIndex0 + 1;
	float scaleFactor = GetScaleFactor(scales[scaleIndex0].timeStamp, scales[scaleIndex1].timeStamp, anim_dt);
	Vector3 finalScale = scales[scaleIndex0].scale * (1 - scaleFactor) + scales[scaleIndex1].scale * scaleFactor;

	return Matrix4::Scale(finalScale);
}