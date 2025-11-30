#pragma once
#include "Environment.h"
#include "nclgl/SpotLight.h"

// "Lamp Post" (https://skfb.ly/oEOxW) by Rares Orza - skypro86 is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).

class LampPost : public Environment {
public:
	LampPost(const std::string path);
	LampPost(const LampPost& other);

	void Update(float dt) override;
	
	void SetModelScale(const Vector3& s) override;

	void RecalculateLightRange(float scale) const { light->CalculateAttenuationValues(lightOffset * scale); }
	SpotLight* GetLight() const { return light; }

private:
	SpotLight* light;
	Vector3 lightOffset;
	Vector3 lightDirection;

};

