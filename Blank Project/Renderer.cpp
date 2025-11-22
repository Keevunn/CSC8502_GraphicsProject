#include "Renderer.h"

#include "Animation.h"

#include "nclgl/Camera.h"
#include "nclgl/SceneNode.h"

#define MODELSDIR "../Models/"
#define ANIMATIONDIR "../Animations/"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	camera = new Camera(-3, 0, Vector3(0, 7, 4));

	shader = new Shader("SkinningVertex.glsl", "TexturedFragment.glsl");
	if (!shader->LoadSuccess()) return;

	model = new Model(MODELSDIR"Robot/RobotModelWithWalkAnim.fbx");
	model->SetModelScale(Vector3(1/100.0f));
	model->SetTransform(Matrix4::Translation(Vector3(0, 0, -20)));
	matTextures = model->GetDiffTex();
	matTextures.push_back(SOIL_load_OGL_texture(MODELSDIR"Robot/Robot_Base_color 5.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0));
	if (matTextures.empty() || matTextures.at(0) == 0) return;

	animation = new Animation(MODELSDIR"Robot/RobotModelWithWalkAnim.fbx", model);
	animator = new Animator(animation);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	init = true;
}

Renderer::~Renderer(void) {
	delete camera;
	delete model;
	delete shader;
	delete animator;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();

	model->Update(dt);
	animator->UpdateAnimation(dt);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(shader);
	UpdateShaderMatrices();

	auto transforms = animator->GetFinalBoneMatrices();
	int j = glGetUniformLocation(shader->GetProgram(), "joints");
	glUniformMatrix4fv(j, transforms.size(), false, (float*)transforms.data());

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
		glBindTexture(GL_TEXTURE_2D, matTextures[0]);

		n->Draw(*this);
	}

	for (auto i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
		DrawNode(*i);

}
