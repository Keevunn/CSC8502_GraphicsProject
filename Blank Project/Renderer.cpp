#include "Renderer.h"
#include "nclgl/Camera.h"
#include "nclgl/MeshAnimation.h"
#include "nclgl/MeshMaterial.h"
#include "Model.h"

#define MODELSDIR "../Models/"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	camera = new Camera(-3, 0, Vector3(0, 1.4f, 4));

	shader = new Shader("BasicTexturedVertex.glsl", "TexturedFragment.glsl");
	if (!shader->LoadSuccess()) return;

	model = new Model(MODELSDIR"Robot/Robot.fbx");
	matTextures = model->GetDiffTex();

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	init = true;
}

Renderer::~Renderer(void) {
	delete camera;
	delete model;
	delete shader;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(shader);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	GLuint texture = matTextures[0];
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	UpdateShaderMatrices();

	for (Mesh* m : model->GetMeshes()) {
		m->Draw();
	}
}
