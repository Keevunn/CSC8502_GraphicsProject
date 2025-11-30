#include "SpotLight.h"

#include <cassert>

SpotLight::SpotLight(const Vector3& direction, const Vector4& diffuseColour, const Vector4& specularColour, float innerCutoff, float outerCutoff)
: Light(Vector3(0), diffuseColour, specularColour), direction(direction), innerCutoff(innerCutoff), outerCutoff(outerCutoff) {
	assert(outerCutoff >= innerCutoff && "Outer cutoff must be greater than or equal to inner cutoff");

	lightVolume = Mesh::LoadFromMeshFile("Cone.msh");

}

Matrix4 SpotLight::GetModelMatrix() const {
	// Original cone 2 units long
	// Tip: (0, 0, 1), Base: (0, 0, -1)

	// Reset tip to origin
	Matrix4 offset = Matrix4::Translation(Vector3(0, 0, -1));
	
	// Scale
	// Range represents Hypotenuse
	// X, Y is scaled by width (opposite)
	// Z is scaled by length (adjacent)
	float length = range * cos(DegToRad(outerCutoff)) * 0.5f; // * 0.5 to bring to 1 unit length 
	float width = range * sin(DegToRad(outerCutoff)); 
	Matrix4 scale = Matrix4::Scale(Vector3(width, width, length));

	// Rotation
	// Currently points down -Z axis
	Vector3 defaultDir = Vector3(0, 0, -1);
	Vector3 rotAxis = Vector3::Cross(direction.Normalised(), defaultDir);
	float dot = Vector3::Dot(direction.Normalised(), defaultDir);
	

	Matrix4 rotation;
	if (rotAxis.Length() > 0.001f) {
		dot = std::clamp(dot, -1.0f, 1.0f);
		float angle = RadToDeg(acos(dot));
		rotation = Matrix4::Rotation(-angle, rotAxis);
	}
	else if (dot < 0)
		rotation = Matrix4::Rotation(180, Vector3(1, 0, 0));

	// Translation
	Matrix4 translation = Matrix4::Translation(position);

	return translation * rotation * scale * offset;
}

void SpotLight::CalculateAttenuationValues(Vector3 offset) { 
	// c++ maths expects radians
	float range = offset.y / cos(DegToRad(outerCutoff)) * 2.0f; // Assuming spotlight points at the ground (y=0)
	auto attenVals = GetSuggestedAttenuationValues(range);
	SetAttenuationValues(range, attenVals.x, attenVals.y, attenVals.z);
}
