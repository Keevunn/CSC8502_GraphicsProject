#pragma once
#include "Light.h"

class DirectionalLight : public Light {
public:
	DirectionalLight() = default;
	DirectionalLight(const Vector3& pDirection, const Vector4& pColour) : direction(pDirection) {
		colour = pColour;
	}

	Vector3		GetDirection() const { return direction; }
	void		SetDirection(const Vector3 dir) { direction = dir; }

protected:
	Vector3 direction;
};

