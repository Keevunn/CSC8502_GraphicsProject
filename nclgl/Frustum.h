#pragma once
#include "Plane.h"
class SceneNode;
class Matrix4;

class Frustum {
public:
	Frustum(void) = default;
	~Frustum(void) = default;

	void FromMatrix(const Matrix4& mat);
	bool InsideFrustum(SceneNode& n);

protected:
	Plane planes[6];
};

