/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Quaternion.h"
#include "Matrix4.h"
#include "Matrix3.h"
#include "Vector3.h"
#include "common.h"

#include <algorithm>
#include <cmath>

Quaternion::Quaternion(void)
{
	x = y = z = 0.0f;
	w = 1.0f;
}

Quaternion::Quaternion(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Quaternion::Quaternion(const Vector3& vector, float w) {
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
	this->w = w;
}

Quaternion::Quaternion(const Matrix4& m) {
	w = sqrt(std::max(0.0f, (1.0f + m.values[0] + m.values[5] + m.values[10]))) * 0.5f;

	if (abs(w) < 0.0001f) {
		x = sqrt(std::max(0.0f, (1.0f + m.values[0] - m.values[5] - m.values[10]))) / 2.0f;
		y = sqrt(std::max(0.0f, (1.0f - m.values[0] + m.values[5] - m.values[10]))) / 2.0f;
		z = sqrt(std::max(0.0f, (1.0f - m.values[0] - m.values[5] + m.values[10]))) / 2.0f;

		x = (float)copysign(x, m.values[9] - m.values[6]);
		y = (float)copysign(y, m.values[2] - m.values[8]);
		z = (float)copysign(z, m.values[4] - m.values[1]);
	}
	else {
		float qrFour = 4.0f * w;
		float qrFourRecip = 1.0f / qrFour;

		x = (m.values[6] - m.values[9]) * qrFourRecip;
		y = (m.values[8] - m.values[2]) * qrFourRecip;
		z = (m.values[1] - m.values[4]) * qrFourRecip;
	}
}

Quaternion::Quaternion(const Matrix3& m) {
	w = sqrt(std::max(0.0f, (1.0f + m.values[0] + m.values[4] + m.values[8]))) * 0.5f;

	float qrFour = 4.0f * w;
	float qrFourRecip = 1.0f / qrFour;

	x = (m.values[5] - m.values[7]) * qrFourRecip;
	y = (m.values[6] - m.values[2]) * qrFourRecip;
	z = (m.values[1] - m.values[3]) * qrFourRecip;
}

Quaternion::~Quaternion(void)
{
}

float Quaternion::Dot(const Quaternion& a, const Quaternion& b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

Matrix4 Quaternion::ToMatrix4() const {
	Matrix4 mat;
	float xx = x * x; float yy = y * y; float zz = z * z;
	float xy = x * y; float xz = x * z; float yz = y * z;
	float wx = w * x; float wy = w * y; float wz = w * z;

	mat.values[0] = 1.0f - 2.0f * (yy + zz);
	mat.values[1] = 2.0f * (xy - wz);
	mat.values[2] = 2.0f * (xz + wy);

	mat.values[4] = 2.0f * (xy + wz);
	mat.values[5] = 1.0f - 2.0f * (xx + zz);
	mat.values[6] = 2.0f * (yz - wx);

	mat.values[8] = 2.0f * (xz - wy);
	mat.values[9] = 2.0f * (yz + wx);
	mat.values[10] = 1.0f - 2.0f * (xx + yy);

	mat.values[15] = 1.0f;
	return mat;
}

void Quaternion::Normalise() {
	float magnitude = sqrt(x * x + y * y + z * z + w * w);

	if (magnitude > 0.0f) {
		float t = 1.0f / magnitude;

		x *= t;
		y *= t;
		z *= t;
		w *= t;
	}
}

void Quaternion::CalculateW() {
	w = 1.0f - (x * x) - (y * y) - (z * z);
	if (w < 0.0f) {
		w = 0.0f;
	}
	else {
		w = -sqrt(w);
	}
}

Quaternion Quaternion::Conjugate() const
{
	return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::Lerp(const Quaternion& from, const Quaternion& to, float by) {
	Quaternion temp = to;

	float dot = Quaternion::Dot(from, to);

	if (dot < 0.0f) {
		temp = -to;
	}

	return (from * (1.0f - by)) + (temp * by);
}

Quaternion Quaternion::Slerp(const Quaternion& from, const Quaternion& to, float by) {
	Quaternion temp = to;

	float dot = Quaternion::Dot(from, to);

	if (dot < 0.0f) {
		temp = -to;
	}

	return (from * (cos(by))) + (to * (1.0f - cos(by)));
}

//http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
//Verified! Different values to above, due to difference between x/z being 'forward'
Vector3 Quaternion::ToEuler() const {
	Vector3 euler;

	float t = x * y + z * w;

	if (t > 0.4999) {
		euler.z = RadToDeg(PI / 2.0f);
		euler.y = RadToDeg(2.0f * atan2(x, w));
		euler.x = 0.0f;

		return euler;
	}

	if (t < -0.4999) {
		euler.z = -RadToDeg(PI / 2.0f);
		euler.y = -RadToDeg(2.0f * atan2(x, w));
		euler.x = 0.0f;
		return euler;
	}

	float sqx = x * x;
	float sqy = y * y;
	float sqz = z * z;

	euler.z = RadToDeg(asin(2 * t)); 
	euler.y = RadToDeg(atan2(2 * y * w - 2 * x * z, 1.0f - 2 * sqy - 2 * sqz)); 
	euler.x = RadToDeg(atan2(2 * x * w - 2 * y * z, 1.0f - 2 * sqx - 2.0f * sqz)); 

	return euler;
}

//http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/
//VERIFIED AS CORRECT - Pitch and roll are changed around as the above uses x as 'forward', whereas we use -z
Quaternion Quaternion::EulerAnglesToQuaternion(float roll, float yaw, float pitch) {
	float cosYaw = (float)cos(DegToRad(yaw * 0.5f));
	float cosPitch = (float)cos(DegToRad(pitch * 0.5f));
	float cosRoll = (float)cos(DegToRad(roll * 0.5f));

	float sinYaw = (float)sin(DegToRad(yaw * 0.5f));
	float sinPitch = (float)sin(DegToRad(pitch * 0.5f));
	float sinRoll = (float)sin(DegToRad(roll * 0.5f));

	Quaternion q;

	q.x = (sinYaw * sinPitch * cosRoll) + (cosYaw * cosPitch * sinRoll);
	q.y = (sinYaw * cosPitch * cosRoll) + (cosYaw * sinPitch * sinRoll);
	q.z = (cosYaw * sinPitch * cosRoll) - (sinYaw * cosPitch * sinRoll);
	q.w = (cosYaw * cosPitch * cosRoll) - (sinYaw * sinPitch * sinRoll);

	return q;
};

Quaternion Quaternion::AxisAngleToQuaterion(const Vector3& vector, float degrees) {
	float theta  = (float)DegToRad(degrees);
	float result = (float)sin(theta / 2.0f);

	return Quaternion((float)(vector.x * result), (float)(vector.y * result), (float)(vector.z * result), (float)cos(theta / 2.0f));
}


Vector3		Quaternion::operator *(const Vector3& a)	const {
	Quaternion newVec = *this * Quaternion(a.x, a.y, a.z, 0.0f) * Conjugate();
	return Vector3(newVec.x, newVec.y, newVec.z);
}