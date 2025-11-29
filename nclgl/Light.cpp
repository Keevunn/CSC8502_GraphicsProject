#include "Light.h"

void Light::SetAttenuationValues(float range, float constant, float linear, float quadratic) {
	this->range = std::max(range, 0.0f);
	this->constant = std::max(constant, 0.0f);
	this->linear = std::max(linear, 0.0f);
	this->quadratic = std::max(quadratic, 0.0f);
}

// Shortcut code taken from https://wiki.ogre3d.org/Light+Attenuation+Shortcut#Code
Vector3 Light::GetSuggestedAttenuationValues(const float range) const {
	/*for (const auto& vals : ATTENVALUES) {
		if (radius >= vals.w) return { vals.x, vals.y, vals.z };
	}
	const auto* last = &ATTENVALUES.back();
	return { last->x, last->y, last->z };*/

	return { 1, 4.5f / range, 75.0f / (range * range) };
}
