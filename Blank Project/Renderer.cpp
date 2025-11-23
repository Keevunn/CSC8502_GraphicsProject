#include "Renderer.h"

#include "Animation.h"

#include "nclgl/Camera.h"
#include "nclgl/HeightMap.h"
#include "nclgl/MeshMaterial.h"
#include "nclgl/SceneNode.h"

#define MODELSDIR "../Models/"
#define ANIMATIONDIR "../Animations/"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	// Load model with walk anim
	/*model = new Model(MODELSDIR"Robot/RobotModelWithWalkAnim.fbx");
	model->SetModelScale(Vector3(1/100.0f));
	model->SetTransform(Matrix4::Translation(Vector3(0, 0, -20)));
	robotTexture = SOIL_load_OGL_texture(MODELSDIR"Robot/Robot_Base_color 5.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	if (robotTexture == 0) return;

	animation = new Animation(MODELSDIR"Robot/RobotModelWithWalkAnim.fbx", model);
	animator = new Animator(animation);
	modelShader = new Shader("SkinningVertex.glsl", "TexturedFragment.glsl");
	if (!modelShader->LoadSuccess()) return;
	*/

	MeshMaterial material("ConcreteFloor.mat"); // for a single mesh (floor)
	const MeshMaterialEntry* matEntry = material.GetMaterialForLayer(0);

	unsigned int flags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS;

	for (const auto& [type, filename] : matEntry->entries) {
		std::string path = TEXTUREDIR + filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, flags);
		if (!texID) return;
		concreteTextures[type] = texID;
	}
	const Vector3 vertexScale = Vector3(1, -0.1f, 1);
	const Vector2 textureScale = Vector2(1 / 100.0f, 1 / 100.0f);
	heightMap = new HeightMap(TEXTUREDIR"/Concrete/PuddleMask.png", vertexScale, textureScale);
	terrainShader = new Shader("TexturedVertex.glsl", "NoisyTexturedFragment.glsl");
	if (!terrainShader->LoadSuccess()) return;

	Vector3 dimensions = heightMap->GetHeightmapSize();
	camera = new Camera(-3, 0, dimensions * Vector3(0.5, 2, 0.5), 200);
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	init = true;
}

Renderer::~Renderer(void) {
	delete camera;

	delete modelShader;
	delete model;

	delete animation;
	delete animator;

	delete terrainShader;
	delete heightMap;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();

	/*model->Update(dt);
	animator->UpdateAnimation(dt);*/
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(terrainShader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(terrainShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, concreteTextures["Diffuse"]);

	glUniform1i(glGetUniformLocation(terrainShader->GetProgram(), "noiseTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, concreteTextures["Noise"]);

	heightMap->Draw();

	// Render model
	/*BindShader(shader);
	UpdateShaderMatrices();

	auto transforms = animator->GetFinalBoneMatrices();
	int j = glGetUniformLocation(shader->GetProgram(), "joints");
	glUniformMatrix4fv(j, transforms.size(), false, (float*)transforms.data());

	DrawNode(model);*/
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Matrix4 modelMatrix = n->GetWorldTransform();
		auto nodeColour = n->GetColour();

		glUniformMatrix4fv(glGetUniformLocation(modelShader->GetProgram(), "modelMatrix"), 1, false, modelMatrix.values);
		//glUniform4fv(glGetUniformLocation(modelShader->GetProgram(), "nodeColour"), 1, (float*)&nodeColour);

		glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "diffuseTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, robotTexture);

		n->Draw(*this);
	}

	for (auto i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
		DrawNode(*i);
}
