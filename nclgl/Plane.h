#pragma once
#include "Vector3.h"

class Plane {
public:
	Plane(void) = default;
	Plane(const Vector3& normal, float distance, bool normalise = false);
	~Plane(void) = default;

	Vector3 GetNormal() const { return normal; }
	void SetNormal(Vector3 n) { normal = n; }

	float GetDistance() const { return distance; }
	void SetDistance(float d) { distance = d; }

	bool SphereInPlane(const Vector3& position, float radius) const;

protected:
	Vector3 normal;
	float distance;
};

