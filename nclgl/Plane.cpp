#include "Plane.h"

Plane::Plane(const Vector3& normal, float distance, bool normalise) : normal(normal), distance(distance) {
	if (normalise) {
		float length = sqrt(Vector3::Dot(normal, normal));
		this->normal = this->normal / length;
		this->distance /= length;
	}
}

bool Plane::SphereInPlane(const Vector3& position, float radius) const {
	float dist = Vector3::Dot(position, normal) + distance;
	return (dist > -radius);
}