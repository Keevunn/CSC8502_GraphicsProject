#include "Renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	shape = Mesh::GenerateQuad();

	basicShader = new Shader("basicVertex.glsl", "colourFragment.glsl");

	if (!basicShader->LoadSuccess()) {
		return;
	}
	init = true;
}

Renderer::~Renderer(void) {
	delete shape;
	delete basicShader;
}

void Renderer::RenderScene() {
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Set clear color to dark grey
	glClear(GL_COLOR_BUFFER_BIT); // Clear the screen to chosen colour

	BindShader(basicShader);
	shape->Draw();
}