#include "Renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	triangle = Mesh::GenerateTriangle();

	texture = SOIL_load_OGL_texture(TEXTUREDIR"brick.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	if (!texture) return;

	shader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	if (!shader->LoadSuccess()) return;

	filtering = true;
	repeating = false;

	init = true;
}

Renderer::~Renderer(void) {
	delete triangle;
	delete shader;
	glDeleteTextures(1, &texture);
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	BindShader(shader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	triangle->Draw();
}

void Renderer::UpdateTextureMatrix(float deg) {
	Matrix4 push = Matrix4::Translation(Vector3(-0.5f, -0.5f, 0));
	Matrix4 pop = Matrix4::Translation(Vector3(0.5f, 0.5f, 0));
	Matrix4 rotation = Matrix4::Rotation(deg, Vector3(0, 0, 1));

	textureMatrix = pop * rotation * push;
}

void Renderer::ToggleRepeating() {
	repeating = !repeating;
	SetTextureRepeating(texture, repeating);
}

void Renderer::ToggleFiltering() {
	filtering = !filtering;
	SetTextureFiltering(texture, filtering);
}
