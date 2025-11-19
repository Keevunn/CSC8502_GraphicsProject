#include "Renderer.h"
#include "Model.h"
#include "nclgl/Camera.h"
#include "nclgl/SceneNode.h"

#define MODELSDIR "../Models/"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	camera = new Camera(-3, 0, Vector3(0, 10, 4));

	shader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	if (!shader->LoadSuccess()) return;

	model = new Model(MODELSDIR"Robot/Robot.fbx");
	model->SetModelScale(Vector3(1.0f / 25.0f));
	model->SetTransform(Matrix4::Translation(Vector3(0, 0, -10)));
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
	model->Update(dt);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(shader);
	

	UpdateShaderMatrices();

	DrawNode(model);
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Matrix4 modelMatrix = n->GetWorldTransform();
		auto nodeColour = n->GetColour();

		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, modelMatrix.values);
		glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)&nodeColour);
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), 1);

		glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, n->GetTexture());

		n->Draw(*this);
	}

	for (auto i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
		DrawNode(*i);

}
