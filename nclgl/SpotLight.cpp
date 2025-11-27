#include "SpotLight.h"

#include <cassert>

SpotLight::SpotLight(const Vector3& position, const Vector4& diffuseColour, const Vector4& specularColour, float innerCutoff, float outerCutoff)
: Light(position, diffuseColour, specularColour), direction(Vector3(0, -1, 0)), innerCutoff(innerCutoff), outerCutoff(outerCutoff) {
	assert(outerCutoff >= innerCutoff && "Outer cutoff must be greater than or equal to inner cutoff");

	lightVolume = Mesh::LoadFromMeshFile("Cone.msh");

	float radius = position.y * std::tan(outerCutoff); // Assuming spotlight points at the ground (y=0)
	auto attenVals = GetSuggestedAttenuationValues(radius);
	SetAttenuationValues(radius, attenVals.x, attenVals.y, attenVals.z);
}
