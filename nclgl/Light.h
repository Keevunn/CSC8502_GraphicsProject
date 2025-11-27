#pragma once
#include "Mesh.h"
#include "Vector4.h"
#include "Vector3.h"

class Light {
public:
	Light() = default;
	Light(const Vector3& position, const Vector4& diffuseColour, const Vector4& specularColour, Mesh* lightVolume = nullptr) 
		: position(position), diffColour(diffuseColour), specColour(specularColour), lightVolume(lightVolume) {}
	~Light() {
		delete lightVolume;
	}

	Vector3		GetPosition() const { return position; }
	void		SetPosition(const Vector3& val) { position = val; }

	float		GetRadius() const { return radius; }
	void		SetRadius(const float val) { radius = val; }

	Vector4		GetDiffuseColour() const { return diffColour; }
	void		SetDiffuseColour(const Vector4& val) { diffColour = val; }

	Vector4		GetSpecularColour() const { return specColour; }
	void		SetSpecularColour(const Vector4& val) { specColour = val; }

	Mesh*		GetLightVolume() const { return lightVolume; }
	void		SetLightVolume(Mesh* m) { lightVolume = m; }

	void		SetAttenuationValues(float radius, float constant, float linear, float quadratic);
	float 		GetConstant() const { return constant; }
	float 		GetLinear() const { return linear; }
	float		GetQuadratic() const { return quadratic; }
	Vector3		GetAttenuationValues() const { return {constant, linear, quadratic}; }

	Vector3 GetSuggestedAttenuationValues(float radius) const;

protected:
	Vector3		position;
	Vector4		diffColour;
	Vector4		specColour;

	Mesh*		lightVolume = nullptr;

	// Attenuation values
	float		radius = 0; // Range of light
	float		constant = 0; // Closer to 0 = brighter light
	float		linear = 0; // Fades faster with distance
	float		quadratic = 0; // Not recommended to change from suggested values (would have to recalculate light)

	const std::vector<Vector4> ATTENVALUES = {
		Vector4(1.0f, 0.0014f, 0.000007f, 3250.0f),
		Vector4(1.0f, 0.007f, 0.0002f, 600.0f),
		Vector4(1.0f, 0.014f, 0.0007f, 325.0f),
		Vector4(1.0f, 0.022f, 0.0019f, 200.0f),
		Vector4(1.0f, 0.027f, 0.0028f, 160.0f),
		Vector4(1.0f, 0.045f, 0.0075f, 100.0f),
		Vector4(1.0f, 0.07f, 0.017f, 65.0f),
		Vector4(1.0f, 0.09f, 0.032f, 50.0f),
		Vector4(1.0f, 0.14f, 0.07f, 32.0f),
		Vector4(1.0f, 0.22f, 0.20f, 20.0f),
		Vector4(1.0f, 0.35f, 0.44f, 13.0f),
		Vector4(1.0f, 0.7f, 1.8f, 7.0f)
	};

};

/*	Suggested attenuation values (https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation)
 *  Range	Constant	Linear	Quadratic
	3250	1.0			0.0014	0.000007
	600		1.0			0.007	0.0002
	325		1.0			0.014	0.0007
	200		1.0			0.022	0.0019
	160		1.0			0.027	0.0028
	100		1.0			0.045	0.0075
	65		1.0			0.07	0.017
	50		1.0			0.09	0.032
	32		1.0			0.14	0.07
	20		1.0			0.22	0.20
	13		1.0			0.35	0.44
	7		1.0			0.7		1.8
*/
