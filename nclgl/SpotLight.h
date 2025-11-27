#pragma once

#include "Light.h"

class SpotLight : public Light {
public:
	SpotLight() = default;
	SpotLight(const Vector3& position, const Vector4& diffuseColour, const Vector4& specularColour, float innerCutoff, float outerCutoff);

	Vector3		GetDirection() const { return direction; }
	void		SetDirection(const Vector3& dir) { direction = dir; }

	float		GetInnerCutoff() const { return innerCutoff; }
	void 		SetInnerCutoff(const float val) { innerCutoff = val; }

	float		GetOuterCutoff() const { return outerCutoff; }
	void 		SetOuterCutoff(const float val) { outerCutoff = val; }

private:
	Vector3 direction;
	// Simulates two cones of light for smoother edges
	float innerCutoff;
	float outerCutoff;
};

