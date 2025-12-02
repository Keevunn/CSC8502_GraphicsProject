#include "LampPost.h"

#include <assimp/postprocess.h>

#include "nclgl/SpotLight.h"

LampPost::LampPost(const std::string path) {
	auto flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GlobalScale | aiProcess_CalcTangentSpace;
	texDir = TEXTUREDIR"/LampPost/";
	Environment::LoadScene(path, "Lamp_Root", flags);

	// Set up light
	lightDirection = Vector3(0.5f, -1, 0);
	light = new SpotLight(
		lightDirection, 
		Vector4(1, 0.65f, 0.25f, 1), // deep orange
		Vector4(1, 0.8f, 0.6f, 1), // paler orange
		20.0f, 
		35.0f); 
	lightOffset = Vector3(1.7f, 4.9f, 0); // Estimation of bulb position in model space
	light->CalculateAttenuationValues(lightOffset);
}

LampPost::LampPost(const LampPost& other) : Environment(other) {
	lightOffset = other.lightOffset;
	light = new SpotLight(*other.light);
	lightDirection = other.lightDirection;
}

void LampPost::Update(float dt) {
	Environment::Update(dt);
	light->SetPosition(worldTransform * lightOffset);

	Vector3 worldDir = (worldTransform * Vector4(lightDirection.x, lightDirection.y, lightDirection.z, 0)).ToVector3();
	light->SetDirection(worldDir.Normalised());
}

void LampPost::SetModelScale(const Vector3& s) {
	Environment::SetModelScale(s);
	RecalculateLightRange(s.y);
}
