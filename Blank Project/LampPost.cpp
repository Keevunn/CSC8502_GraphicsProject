#include "LampPost.h"

#include <assimp/postprocess.h>

#include "nclgl/SpotLight.h"

LampPost::LampPost(const std::string path) {
	auto flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GlobalScale | aiProcess_CalcTangentSpace;
	texDir = TEXTUREDIR"/LampPost/";
	Environment::LoadScene(path, "Lamp_Root", flags);

	// Set up light
	light = new SpotLight(
		Vector3(0, 0, 0), // position set in Update()
		Vector4(1, 0.65f, 0.25f, 1), // deep orange
		Vector4(1, 0.8f, 0.6f, 1), // paler orange
		20.0f, // inner cutoff
		35.0f); // outer cutoff
	lightOffset = Vector3(1.7f, 4.9f, 0); // Estimation of bulb position in model space
	light->CalculateAttenuationValues(lightOffset);
}

LampPost::LampPost(const LampPost& other) : Environment(other) {
	lightOffset = other.lightOffset;
	light = new SpotLight(*other.light);
}

void LampPost::Update(float dt) {
	Environment::Update(dt);
	light->SetPosition(worldTransform * lightOffset);
}

void LampPost::SetModelScale(const Vector3& s) {
	Environment::SetModelScale(s);
	RecalculateLightRange(s.y);
}
