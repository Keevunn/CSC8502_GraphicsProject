#pragma once
#include "Environment.h"

// "Lamp Post" (https://skfb.ly/oEOxW) by Rares Orza - skypro86 is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).

class LampPost : public Environment {
public:
	LampPost(const std::string path);
	LampPost(const LampPost& other);

	void Update(float dt) override;

	SpotLight& GetLight() const { return *light; }

private:
	SpotLight* light;
	Vector3 lightOffset;

};

