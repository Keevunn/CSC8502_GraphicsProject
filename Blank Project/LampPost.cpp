#include "LampPost.h"

#include <assimp/postprocess.h>

LampPost::LampPost(const std::string path) {
	auto flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GlobalScale | aiProcess_CalcTangentSpace;
	texDir = TEXTUREDIR"/LampPost/";
	Environment::LoadScene(path, "Lamp_Root", flags);

	// Set up light
	light = new SpotLight(
		Vector3(0, 0, 0), // position set in Update()
		Vector4(1, 0.65f, 0.25f, 1), // deep orange
		Vector4(1, 0.8f, 0.6f, 1), // paler orange
		15.0f, // inner cutoff
		25.0f); // outer cutoff
	lightOffset = Vector3(1.7f, 4.9f, 0); // Estimation of bulb position in model space
}

LampPost::LampPost(const LampPost& other) {
	mesh = other.mesh;
	colour = other.colour;
	texture = other.texture;
	material = other.material;

	transform = other.transform;
	modelScale = other.modelScale;
	
	distanceFromCamera = other.distanceFromCamera;
	boundingRadius = other.boundingRadius;

	lightOffset = other.lightOffset;
	light = new SpotLight(*other.light);

	name = other.name;

	for (unsigned int i{}; i < other.children.size(); ++i)
		AddChild(new SceneNode(*other.children[i]));
}

void LampPost::Update(float dt) {
	Environment::Update(dt);

	float dist = 0.1f * dt; // Moves 0.1m per sec

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT))
		lightOffset.x += dist;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))
		lightOffset.x -= dist;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_UP))
		lightOffset.z += dist;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN))
		lightOffset.z -= dist;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_PLUS))
		lightOffset.y += dist;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_MINUS))
		lightOffset.y -= dist;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_RETURN))
		std::cout << "Light offset: " << lightOffset << "\n";

	light->SetPosition(worldTransform * lightOffset);
}
