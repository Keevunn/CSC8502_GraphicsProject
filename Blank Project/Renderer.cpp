#include "Renderer.h"

#include "Animation.h"
#include "Environment.h"

#include "nclgl/Camera.h"
#include "nclgl/HeightMap.h"
#include "nclgl/MeshMaterial.h"
#include "nclgl/SceneNode.h"

#define MODELSDIR "../Models/"
#define ANIMATIONDIR "../Animations/"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	// Load model with walk anim
	model = new RobotModel(MESHDIR"Robot.fbx");
	//model->SetModelScale(Vector3(1/100.0f));
	//model->SetTransform(Matrix4::Translation(Vector3(0, 0, -20)));
	robotTexture = SOIL_load_OGL_texture(MODELSDIR"Robot/Robot_Base_color 5.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	if (robotTexture == 0) return;
	if (model->HasMaterials()) std::cout << "Robot found materials \n";

	animation = new Animation(MODELSDIR"Robot/RobotModelWithWalkAnim.fbx", model);
	animator = new Animator(animation);
	modelShader = new Shader("SkinningVertex.glsl", "RobotFrag.glsl");
	emissiveShader = new Shader("EmissiveVertex.glsl", "EmissiveFrag.glsl");
	if (!modelShader->LoadSuccess() || !emissiveShader->LoadSuccess()) return;
	
	
	// Position treated as direction
	sun = new DirectionalLight(Vector3(0.2f, -1.0f, -0.3f), Vector4(0.7f, 0.7f, 0.75f, 1));
	sunShader = new Shader("combineVert.glsl", "DirectionalLightFrag.glsl");

	unsigned int flags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS;

	// Construction yard skybox
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"/ConstructionYard/px.png", TEXTUREDIR"/ConstructionYard/nx.png",
		TEXTUREDIR"/ConstructionYard/py.png", TEXTUREDIR"/ConstructionYard/ny.png",
		TEXTUREDIR"/ConstructionYard/pz.png", TEXTUREDIR"/ConstructionYard/nz.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	quad = Mesh::GenerateQuad();
	skyboxShader = new Shader("SkyboxVertex.glsl", "SkyboxFragment.glsl");
	if (!skyboxShader->LoadSuccess()) return;

	// Concrete ground
	MeshMaterial material("ConcreteFloor.mat"); // for a single mesh (floor)
	const MeshMaterialEntry* matEntry = material.GetMaterialForLayer(0);

	for (const auto& [type, filename] : matEntry->entries) {
		std::string path = TEXTUREDIR + filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, flags);
		if (!texID) return;
		concreteTextures[type] = texID;
	}

	const Vector3 vertexScale = Vector3(2, -0.1f, 2);
	const Vector2 textureScale = Vector2(1 / 16.0f, 1 / 16.0f);
	heightMap = new HeightMap(TEXTUREDIR"/Concrete/PuddleMask.png", vertexScale, textureScale);
	//terrainShader = new Shader("TexturedVertex.glsl", "NoisyTexturedFragment.glsl");
	//if (!terrainShader->LoadSuccess()) return;
	Vector3 dimensions = heightMap->GetHeightmapSize();


	environmentShader = new Shader("BumpVertex.glsl", "NoisyTexturedFragment.glsl");
	if (!environmentShader->LoadSuccess()) return;

	// Load city scene
	environment = new Environment(MESHDIR"/Factory/scene.gltf"); 

	Matrix4 cityTransformation =	Matrix4::Translation(dimensions * Vector3(0.65f, 0, 0.2f));
	environment->SetTransform(cityTransformation);
	environment->SetModelScale(Vector3(35));
	
	// Point light and combine shaders
	pointLightShader = new Shader("pointLightVert.glsl", "pointLightFrag.glsl");
	combineShader = new Shader("combineVert.glsl", "combineFrag.glsl");
	if (!pointLightShader->LoadSuccess() || !combineShader->LoadSuccess()) return;
	lightVolume = Mesh::LoadFromMeshFile("Sphere.msh");
	
	camera = new Camera(-3, 0, dimensions * Vector3(0.5, 2, 0.5), 200);

	// FBOs
	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &pointLightFBO);

	GLenum buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1}; // stores textures samples and normals

	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex);
	GenerateScreenTexture(bufferNormalTex);
	GenerateScreenTexture(lightDiffuseTex);
	GenerateScreenTexture(lightSpecularTex);

	// First pass 
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bufferNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glDrawBuffers(2, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return;

	// Second pass
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightSpecularTex, 0);
	glDrawBuffers(2, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	init = true;
}

Renderer::~Renderer(void) {
	delete camera;

	delete modelShader;
	delete model;

	delete animator;

	delete skyboxShader;
	delete quad;

	delete environmentShader;
	delete environment;
	delete lightVolume;

	delete pointLightShader;
	delete combineShader;

	//delete terrainShader;
	delete heightMap;

	glDeleteTextures(1, &bufferColourTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &lightDiffuseTex);
	glDeleteTextures(1, &lightSpecularTex);

	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &pointLightFBO);
}

void Renderer::RenderScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	

	glClearColor(0.2f, 0.2f, 0.2f, 1); // In case the skybox fails
	DrawSkybox();

	FillBuffers();
	DrawLights();
	CombineBuffers();

	BindShader(emissiveShader);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthFunc(GL_LEQUAL);

	DrawNode(model, emissiveShader);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();

	environment->Update(dt);

	model->Update(dt);
	animator->UpdateAnimation(dt);
}

void Renderer::GenerateScreenTexture(GLuint& into, bool depth) {
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::FillBuffers() {
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);

	glClearColor(0, 0, 0, 0); // Clear to transparent black so combine shader discards empty pixels
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	DrawHeightMap();

	BindShader(environmentShader);
	DrawNode(environment, environmentShader);

	DrawRobot();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawLights() {
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	auto camPos_Vec3 = camera->GetPosition();
	auto camPos = reinterpret_cast<float*>(&camPos_Vec3);

	DrawSun(invViewProj, camPos);
	DrawPointLights(invViewProj, camPos);

	glDisable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::CombineBuffers() {
	BindShader(combineShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseLight"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "specularLight"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);
	quad->Draw();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	BindShader(skyboxShader);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightMap() {
	BindShader(environmentShader);

	glUniform1i(glGetUniformLocation(environmentShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, concreteTextures["Diffuse"]);

	glUniform1i(glGetUniformLocation(environmentShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, concreteTextures["Bump"]);

	glUniform1i(glGetUniformLocation(environmentShader->GetProgram(), "noiseTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, concreteTextures["Noise"]);

	glUniform1i(glGetUniformLocation(environmentShader->GetProgram(), "roughnessTex"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, concreteTextures["Roughness"]);

	glUniform1i(glGetUniformLocation(environmentShader->GetProgram(), "useNoise"), 1);
	int hasRoughness = concreteTextures["Roughness"] > 0 ? 1 : 0;
	glUniform1i(glGetUniformLocation(environmentShader->GetProgram(), "hasRoughness"), hasRoughness);
	int hasBump = concreteTextures["Bump"] > 0 ? 1 : 0;
	glUniform1i(glGetUniformLocation(environmentShader->GetProgram(), "hasBump"), hasBump);

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::DrawNode(SceneNode* n, Shader* shader) {
	if (n->GetMesh()) {
		modelMatrix = n->GetWorldTransform();
		auto mat = n->GetMaterial();

		auto nodeColour = n->GetColour();
		glUniform4fv(glGetUniformLocation(modelShader->GetProgram(), "nodeColour"), 1, (float*)&nodeColour);

		glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "bumpTex"), 1);
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "reflectionTex"), 2);
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "alphaTex"), 3);
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "roughnessTex"), 4);
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "metallicTex"), 5);
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "emissiveTex"), 6);

		glUniform1i(glGetUniformLocation(shader->GetProgram(), "useNoise"), 0);
		
		int hasBump = mat.bumpID > 0 ? 1 : 0;
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "hasBump"), hasBump);
		int hasRoughness = (mat.specularID > 0) ? 1 : 0;
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "hasRoughness"), hasRoughness);
		int hasOpacity = mat.alphaID > 0 ? 1 : 0;
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "hasOpacity"), hasOpacity);

		UpdateShaderMatrices();
		n->Draw(*this);
	}

	for (auto i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
		DrawNode(*i, shader);
}

void Renderer::DrawRobot() {
	BindShader(modelShader);

	auto transforms = animator->GetFinalBoneMatrices();
	glUniformMatrix4fv(glGetUniformLocation(modelShader->GetProgram(), "joints"), transforms.size(), false, (float*)transforms.data());

	DrawRobotNode(model);
}

void Renderer::DrawRobotNode(SceneNode* n) {
	if (n->GetMesh()) {
		modelMatrix = n->GetWorldTransform();
		auto mat = n->GetMaterial();

		if (mat.diffuseID == 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, robotTexture);
		}

		glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "diffuseTex"), 0);
		glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "bumpTex"), 1);
		glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "reflectionTex"), 2);
		glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "alphaTex"), 3);
		glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "roughnessTex"), 4);
		glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "metallicTex"), 5);

		UpdateShaderMatrices();
		n->Draw(*this);
	}

	for (auto i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
		DrawRobotNode(*i);
}

void Renderer::DrawSun(Matrix4 invViewProj, float* camPos) {
	BindShader(sunShader);

	glUniform1i(glGetUniformLocation(sunShader->GetProgram(), "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(sunShader->GetProgram(), "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform3fv(glGetUniformLocation(sunShader->GetProgram(), "cameraPos"), 1, camPos);

	glUniform2f(glGetUniformLocation(sunShader->GetProgram(), "pixelSize"), 1.0f / width, 1.0f / height);

	
	glUniformMatrix4fv(glGetUniformLocation(sunShader->GetProgram(), "inverseProjView"), 1, false, invViewProj.values);

	SetShaderLight(*sun);
	quad->Draw();
}

void Renderer::DrawPointLights(Matrix4 invViewProj, float* camPos) {
	BindShader(pointLightShader);

	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	glUniform1i(glGetUniformLocation(pointLightShader->GetProgram(), "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(pointLightShader->GetProgram(), "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform3fv(glGetUniformLocation(pointLightShader->GetProgram(), "cameraPos"), 1, (float*)&camPos);

	glUniform2f(glGetUniformLocation(pointLightShader->GetProgram(), "pixelSize"), 1.0f / width, 1.0f / height);

	glUniformMatrix4fv(glGetUniformLocation(pointLightShader->GetProgram(), "inverseProjView"), 1, false, invViewProj.values);

	for (const auto& light : pointLights) {
		SetShaderLight(light); // TODO the light volume should be attached to the object
		lightVolume->Draw();
	}

	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
}
