#include "Light.h"

void Light::SetAttenuationValues(float radius, float constant, float linear, float quadratic) {
	this->radius = std::max(radius, 0.0f);
	this->constant = std::max(constant, 0.0f);
	this->linear = std::max(linear, 0.0f);
	this->quadratic = std::max(quadratic, 0.0f);
}

Vector3 Light::GetSuggestedAttenuationValues(const float radius) const {
	for (const auto& vals : ATTENVALUES) {
		if (radius >= vals.w) return { vals.x, vals.y, vals.z };
	}
	const auto* last = &ATTENVALUES.back();
	return { last->x, last->y, last->z };
}
