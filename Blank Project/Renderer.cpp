#include "Renderer.h"

#include "Factory.h"
#include "FactoryScene.h"
#include "KittenModel.h"
#include "NatureScene.h"

#include "nclgl/Camera.h"
#include "nclgl/DirectionalLight.h"
#include "nclgl/HeightMap.h"
#include "nclgl/MeshMaterial.h"
#include "nclgl/SceneNode.h"
#include "nclgl/SpotLight.h"

#define MODELSDIR "../Models/"
#define ANIMATIONDIR "../Animations/"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();

	// Load all animations once
	globalKittenModel = std::make_shared<KittenModel>(MODELSDIR"KittenSadWalk.fbx", Vector3(), "SadWalk");
	globalKittenModel->LoadAnimationFromFile(MODELSDIR"KittenDancing1.fbx", "Dancing1");
	globalKittenModel->LoadAnimationFromFile(MODELSDIR"KittenDancing2.fbx", "Dancing2");
	globalKittenModel->LoadAnimationFromFile(MODELSDIR"KittenSittingPose.fbx", "SittingPose");
	globalKittenModel->LoadAnimationFromFile(MODELSDIR"KittenSwingingLegs.fbx", "SwingingLegs");
	globalKittenModel->LoadAnimationFromFile(MODELSDIR"KittenTreadingWater.fbx", "TreadingWater");

	// Scenes
	factoryScene = new FactoryScene(*this);
	natureScene = new NatureScene(*this);
	if (!natureScene->LoadSuccess() || !factoryScene->LoadSuccess()) return;

	currentScene = natureScene;

	// Shaders
	// Sun
	sunShader = new Shader("combineVert.glsl", "DirectionalLightFrag.glsl");
	if (!sunShader->LoadSuccess()) return;
	// Skybox
	skyboxShader = new Shader("SkyboxVertex.glsl", "SkyboxFragment.glsl");
	if (!skyboxShader->LoadSuccess()) return;
	// Concrete ground
	concreteShader = new Shader(
		"BasicTessellationVert.glsl",
		"ConcreteFloorFrag.glsl",
		"", // No geometry shader
		"BasicTCS.glsl",
		"ConcreteFloorTES.glsl"); 
	waterShader = new Shader(
		"BasicTessellationVert.glsl",
		"WaterFrag.glsl",
		"", // No geometry shader
		"BasicTCS.glsl",
		"WaterTES.glsl");
	natureShader = new Shader(
		"BasicTessellationVert.glsl",
		"NatureSceneTerrainFrag.glsl",
		"", // No geometry shader
		"BasicTCS.glsl",
		"NatureSceneTerrainTES.glsl");
	if (!concreteShader->LoadSuccess() || !waterShader->LoadSuccess() || !natureShader->LoadSuccess()) return;
	// Scene shaders
	environmentShader = new Shader("BumpVertex.glsl", "PBRFrag.glsl");
	//waterShader = new Shader("WaterVert.glsl", "WaterFrag.glsl");
	animationShader = new Shader("SkinningVertex.glsl", "PBRFrag.glsl");
	spotLightShader = new Shader("BasicMatrixVertex.glsl", "SpotLightFrag.glsl");
	if (!environmentShader->LoadSuccess() || !animationShader->LoadSuccess() || !spotLightShader->LoadSuccess()) return;
	// Combine Shaders
	combineShader = new Shader("combineVert.glsl", "combineFrag.glsl");
	if (!combineShader->LoadSuccess()) return;

	// FBOs
	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &pointLightFBO);

	GLenum buffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	// Colour, Normal, Emissive textures
	GLenum lightBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1}; // Diffuse, Specular

	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex);
	GenerateScreenTexture(bufferNormalTex);
	GenerateScreenTexture(bufferEmissiveTex);
	GenerateScreenTexture(lightDiffuseTex);
	GenerateScreenTexture(lightSpecularTex);

	// First pass 
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bufferNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, bufferEmissiveTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glDrawBuffers(3, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return;

	// Second pass
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightSpecularTex, 0);
	glDrawBuffers(2, lightBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	init = true;
}

// TODO blend maps for scene 2

Renderer::~Renderer(void) {
	delete skyboxShader;
	delete sunShader;
	delete pointLightShader;
	delete spotLightShader;
	delete concreteShader;
	delete waterShader;
	delete environmentShader;
	delete animationShader;
	delete combineShader;

	glDeleteTextures(1, &bufferColourTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &bufferEmissiveTex);
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

}

void Renderer::UpdateScene(float dt) {
	currentScene->Update(dt);
	viewMatrix = currentScene->GetCamera()->BuildViewMatrix();

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_TAB)) {
		if (currentScene == factoryScene) currentScene = natureScene;
		else currentScene = factoryScene;
		currentScene->ResetPathTime();
	}
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
	
	/*projMatrix = factoryScene->GetDefaultProjMatrix();
	factoryScene->RenderGeometry(*this);*/
	projMatrix = currentScene->GetDefaultProjMatrix();
	currentScene->RenderGeometry(*this);

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

	DrawSun();
	currentScene->RenderLights(*this);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

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

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "emissiveTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferEmissiveTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseLight"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "specularLight"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "depthTex"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "normTex"), 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "skyboxTex"), 6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, currentScene->GetSkyboxTex());

	Matrix4 cameraView = currentScene->GetCamera()->BuildViewMatrix();
	Matrix4 invProjView = (currentScene->GetDefaultProjMatrix() * cameraView).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(combineShader->GetProgram(), "inverseProjView"), 1, false, invProjView.values);

	auto camPos = currentScene->GetCamera()->GetPosition();
	glUniform3fv(glGetUniformLocation(combineShader->GetProgram(), "cameraPos"), 1, (float*)&camPos);

	glUniform1f(glGetUniformLocation(combineShader->GetProgram(), "fogDensity"), currentScene->GetFogDensity());

	quad->Draw();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	BindShader(skyboxShader);

	glUniform1i(glGetUniformLocation(skyboxShader->GetProgram(), "cubeTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, currentScene->GetSkyboxTex());

	//projMatrix = factoryScene->GetDefaultProjMatrix();
	projMatrix = currentScene->GetDefaultProjMatrix();
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}


void Renderer::DrawSpotLights(std::vector<SpotLight*> lights) {
	BindShader(spotLightShader);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	auto camPos_Vec3 = currentScene->GetCamera()->GetPosition();
	auto camPos = reinterpret_cast<float*>(&camPos_Vec3);
	BindLightUniforms(spotLightShader->GetProgram(), invViewProj, camPos);

	glCullFace(GL_FRONT);

	for (const auto& l : lights) {
		modelMatrix = l->GetModelMatrix();
		UpdateShaderMatrices();

		SetShaderLight(*l);
		l->GetLightVolume()->Draw();
	}
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	glCullFace(GL_BACK);
}

void Renderer::DrawConcreteFloor(HeightMap* floor, std::unordered_map<std::string, GLuint> floorTextures) {
	BindShader(concreteShader);
	GLuint programLocation = concreteShader->GetProgram();

	glPatchParameteri(GL_PATCH_VERTICES, 3);

	auto BindTex = [&](const char* name, int slot, GLuint texID) {
		glUniform1i(glGetUniformLocation(programLocation, name), slot);
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, texID);
		};

	BindTex("diffuseTex", 0, floorTextures["Diffuse"]);
	BindTex("bumpTex", 1, floorTextures["Bump"]);
	BindTex("aoTex", 2, floorTextures["AO"]);
	BindTex("displacementTex", 3, floor->GetTexture());
	BindTex("roughnessTex", 4, floorTextures["Roughness"]);
	BindTex("noiseTex", 5, floorTextures["Noise"]);

	// Tesselation level
	// Higher = smoother geometry, more expensive
	glUniform1f(glGetUniformLocation(programLocation, "tessLevel"), 32.0f);

	// Displacement Strength
	// Height of bumps in world units
	glUniform1f(glGetUniformLocation(programLocation, "displacementStrength"), 0.04);
	glUniform1i(glGetUniformLocation(programLocation, "texWidth"), floor->GetTexWidth());

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	// Wireframe for debugging
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	floor->Draw();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		modelMatrix = n->GetWorldTransform();
		UpdateShaderMatrices();

		n->Draw(*this);
	}

	for (auto i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
		DrawNode(*i);
}

void Renderer::DrawFactory(Factory* factory) {
	BindShader(environmentShader);
	GLuint programLocation = environmentShader->GetProgram();

	glUniform1i(glGetUniformLocation(programLocation, "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(programLocation, "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "alphaTex"), 2);
	glUniform1i(glGetUniformLocation(programLocation, "roughnessTex"), 3);
	glUniform1i(glGetUniformLocation(programLocation, "metallicTex"), 4);
	glUniform1i(glGetUniformLocation(programLocation, "emissiveTex"), 5);

	glUniform1i(glGetUniformLocation(programLocation, "hasMixedPBR"), 1); //MetallicRoughness Map

	glUniform1i(glGetUniformLocation(programLocation, "hasBump"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "hasOpacity"), 0);
	glUniform1i(glGetUniformLocation(programLocation, "hasRoughness"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "hasMetallic"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "hasEmissive"), 0);

	DrawNode(factory);
}

void Renderer::DrawLampPosts(const ModelPool<LampPost>* lampPosts) {
	BindShader(environmentShader);
	GLuint programLocation = environmentShader->GetProgram();

	glUniform1i(glGetUniformLocation(programLocation, "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(programLocation, "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "alphaTex"), 2);
	glUniform1i(glGetUniformLocation(programLocation, "roughnessTex"), 3);
	glUniform1i(glGetUniformLocation(programLocation, "metallicTex"), 4);
	glUniform1i(glGetUniformLocation(programLocation, "emissiveTex"), 5);

	glUniform1i(glGetUniformLocation(programLocation, "hasMixedPBR"), 0); //MetallicRoughness Map

	glUniform1i(glGetUniformLocation(programLocation, "hasBump"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "hasOpacity"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "hasRoughness"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "hasMetallic"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "hasEmissive"), 0);

	const auto pool = lampPosts->GetPool();
	for (const auto& lampPost : pool)
		DrawNode(lampPost);
}

void Renderer::DrawRobots(const AnimatedModelPool<RobotModel>* robots) {
	BindShader(animationShader);
	GLuint programLocation = animationShader->GetProgram();

	glUniform1i(glGetUniformLocation(programLocation, "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(programLocation, "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "alphaTex"), 2);
	glUniform1i(glGetUniformLocation(programLocation, "roughnessTex"), 3);
	glUniform1i(glGetUniformLocation(programLocation, "metallicTex"), 4);
	glUniform1i(glGetUniformLocation(programLocation, "emissiveTex"), 5);

	glUniform1i(glGetUniformLocation(programLocation, "hasBump"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "hasOpacity"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "hasRoughness"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "hasMetallic"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "hasEmissive"), 1);

	glUniform1f(glGetUniformLocation(programLocation, "emissionIntensity"), 0.5);
	glUniform1f(glGetUniformLocation(programLocation, "time"), currentTime);

	const auto pool = robots->GetPool();
	for (const auto& robot : pool) {
		auto& transforms = robot->GetFinalBoneMatrices();
		glUniformMatrix4fv(glGetUniformLocation(programLocation, "joints"), transforms.size(), false, (float*)transforms.data());
		DrawNode(robot);
	}
}

void Renderer::DrawKittens(const AnimatedModelPool<KittenModel>* kittens) {
	const auto pool = kittens->GetPool();
	DrawKittens(pool);
}

void Renderer::DrawKittens(const std::vector<KittenModel*>& kittens) {
	BindShader(animationShader);
	GLuint programLocation = animationShader->GetProgram();

	glUniform1i(glGetUniformLocation(programLocation, "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(programLocation, "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(programLocation, "alphaTex"), 2);
	glUniform1i(glGetUniformLocation(programLocation, "roughnessTex"), 3);
	glUniform1i(glGetUniformLocation(programLocation, "metallicTex"), 4);
	glUniform1i(glGetUniformLocation(programLocation, "emissiveTex"), 5);

	glUniform1i(glGetUniformLocation(programLocation, "hasBump"), 0);
	glUniform1i(glGetUniformLocation(programLocation, "hasOpacity"), 0);
	glUniform1i(glGetUniformLocation(programLocation, "hasRoughness"), 0);
	glUniform1i(glGetUniformLocation(programLocation, "hasMetallic"), 0);
	glUniform1i(glGetUniformLocation(programLocation, "hasEmissive"), 0);

	glUniform1f(glGetUniformLocation(programLocation, "emissionIntensity"), 0);
	glUniform1f(glGetUniformLocation(programLocation, "time"), currentTime);

	
	for (KittenModel* kitten : kittens) {
		auto& transforms = kitten->GetFinalBoneMatrices();
		glUniformMatrix4fv(glGetUniformLocation(programLocation, "joints"), transforms.size(), false, (float*)transforms.data());
		DrawNode(kitten);
	}
}

void Renderer::DrawTerrain(HeightMap* terrain, GLuint blendMapID, std::unordered_map<string, GLuint> baseTextures, std::unordered_map<string, GLuint> sandTextures, 
                           std::unordered_map<string, GLuint> grassTextures, std::unordered_map<string, GLuint> rockTextures) {
	BindShader(natureShader);
	GLuint programLocation = natureShader->GetProgram();
	glPatchParameteri(GL_PATCH_VERTICES, 3);

	auto BindTex = [&](const char* name, int slot, GLuint texID) {
		glUniform1i(glGetUniformLocation(programLocation, name), slot);
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, texID);
		};

	BindTex("displacementTex", 0, terrain->GetTexture());
	BindTex("blendMap", 1, blendMapID);

	// Bind base textures
	BindTex("baseDiffuseTex", 2, baseTextures["Diffuse"]);
	BindTex("baseBumpTex", 3, baseTextures["Bump"]);
	BindTex("baseAOTex", 4, baseTextures["AO"]);
	BindTex("baseRoughnessTex", 5, baseTextures["Roughness"]);
	BindTex("baseHeightMap", 6, baseTextures["Displacement"]);

	// Bind sand textures
	BindTex("sandDiffuseTex", 7, sandTextures["Diffuse"]);
	BindTex("sandBumpTex", 8, sandTextures["Bump"]);
	BindTex("sandAOTex", 9, sandTextures["AO"]);
	BindTex("sandRoughnessTex", 10, sandTextures["Roughness"]);
	BindTex("sandHeightMap", 11, sandTextures["Displacement"]);

	// Bind grass textures
	BindTex("grassDiffuseTex", 12, grassTextures["Diffuse"]);
	BindTex("grassBumpTex", 13, grassTextures["Bump"]);
	BindTex("grassAOTex", 14, grassTextures["AO"]);
	BindTex("grassRoughnessTex", 15, grassTextures["Roughness"]);
	BindTex("grassHeightMap", 16, grassTextures["Displacement"]);

	// Bind rock textures
	BindTex("rockDiffuseTex", 17, rockTextures["Diffuse"]);
	BindTex("rockBumpTex", 18, rockTextures["Bump"]);
	BindTex("rockAOTex", 19, rockTextures["AO"]);
	BindTex("rockRoughnessTex", 20, rockTextures["Roughness"]);
	BindTex("rockHeightMap", 21, rockTextures["Displacement"]);

	// Displacement Strength
	// Height of bumps in world units
	glUniform1f(glGetUniformLocation(programLocation, "displacementStrength"), 25);
	glUniform1i(glGetUniformLocation(programLocation, "texWidth"), terrain->GetTexWidth());
	glUniform1f(glGetUniformLocation(programLocation, "texScale"), 60);
	glUniform1f(glGetUniformLocation(programLocation, "tessLevel"), 32.0f);

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	// Wireframe for debugging
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	terrain->Draw();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::DrawWater(GLuint waterTex, GLuint skybox, HeightMap* waterMap) {
	BindShader(waterShader);
	GLuint programLocation = waterShader->GetProgram();
	glPatchParameteri(GL_PATCH_VERTICES, 3);

	auto camPos_Vec3 = natureScene->GetCamera()->GetPosition();
	auto camPos = reinterpret_cast<float*>(&camPos_Vec3);
	glUniform3fv(glGetUniformLocation(programLocation, "cameraPos"), 1, camPos);

	glUniform1i(glGetUniformLocation(programLocation, "opacityTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glUniform1i(glGetUniformLocation(programLocation, "cubeTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);

	glUniform1f(glGetUniformLocation(programLocation, "tessLevel"), 32.0f);

	glUniform1f(glGetUniformLocation(programLocation, "time"), currentTime);

	Vector4 blue = Vector4(99.0f / 255.0f, 216.0f / 255.0f, 1, 1);
	glUniform4fv(glGetUniformLocation(programLocation, "colour"), 1, (float*)&blue);

	Vector3 mapSize = waterMap->GetHeightmapSize();
	Vector3 pos = Vector3(0, 5, 0); //5 units high
	modelMatrix = Matrix4::Translation(pos);
	UpdateShaderMatrices();

	waterMap->Draw();
}

void Renderer::DrawSun() {
	BindShader(sunShader);
	GLuint programLocation = sunShader->GetProgram();

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	auto camPos_Vec3 = currentScene->GetCamera()->GetPosition();
	auto camPos = reinterpret_cast<float*>(&camPos_Vec3);
	BindLightUniforms(sunShader->GetProgram(), invViewProj, camPos);

	glUniform1i(glGetUniformLocation(programLocation, "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(programLocation, "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform3fv(glGetUniformLocation(programLocation, "cameraPos"), 1, camPos);

	glUniform2f(glGetUniformLocation(programLocation, "pixelSize"), 1.0f / width, 1.0f / height);


	glUniformMatrix4fv(glGetUniformLocation(programLocation, "inverseProjView"), 1, false, invViewProj.values);

	SetShaderLight(*currentScene->GetSun());
	quad->Draw();
}

void Renderer::BindLightUniforms(GLuint programLocation, Matrix4 invViewProj, float* camPos) const {
	glUniform1i(glGetUniformLocation(programLocation, "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(programLocation, "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform3fv(glGetUniformLocation(programLocation, "cameraPos"), 1, camPos);

	glUniform2f(glGetUniformLocation(programLocation, "pixelSize"), 1.0f / width, 1.0f / height);

	glUniformMatrix4fv(glGetUniformLocation(programLocation, "inverseProjView"), 1, false, invViewProj.values);

}

/*void Renderer::DrawPointLights(Matrix4 invViewProj, float* camPos) {
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
		SetShaderLight(light);
		lightVolume->Draw();
	}

	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
}*/
