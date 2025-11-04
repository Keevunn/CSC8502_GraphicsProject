#include "Renderer.h"
#include "nclgl/CubeRobot.h"
#include "nclgl/Camera.h"
#include <algorithm>

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh");
	quad = Mesh::GenerateQuad();

	camera = new Camera(0, 0, Vector3(0,100,750));

	shader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	texture = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	if (!shader->LoadSuccess() || !texture) return;

	projMatrix = Matrix4::Perspective(1, 10000,
		static_cast<float>(width) / static_cast<float>(height), 45);

	root = new SceneNode();
	for (int i{}; i < 5; ++i) {
		SceneNode* s = new SceneNode();
		s->SetColour(Vector4(1, 1, 1, 0.5f));
		s->SetTransform(Matrix4::Translation(Vector3(0, 100, -200+100*i)));
		s->SetModelScale(Vector3(100, 100, 100));
		s->SetBoundingRadius(100.0f);
		s->SetMesh(quad);
		s->SetTexture(texture);
		root->AddChild(s);
	}
	root->AddChild(new CubeRobot(cube));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	init = true;
}

Renderer::~Renderer(void) {
	delete root;
	delete shader;
	delete camera;
	delete cube;
	delete quad;
	glDeleteTextures(1, &texture);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	root->Update(dt);
}

void Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	BindShader(shader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);

	DrawNodes();

	ClearNodeLists();
}

void Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) transparentNodeList.push_back(from); 
		else opaqueNodeList.push_back(from);
	}

	for (auto i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) 
		BuildNodeLists(*i);
}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), SceneNode::CompareByCameraDistance);
	std::sort(opaqueNodeList.begin(), opaqueNodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	opaqueNodeList.clear();
}

void Renderer::DrawNodes() {
	for (const auto& i : opaqueNodeList)
		DrawNode(i);
	for (const auto& i : transparentNodeList)
		DrawNode(i);
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());

		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, model.values);
		glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());

		texture = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), texture);

		n->Draw(*this);
	}
}
