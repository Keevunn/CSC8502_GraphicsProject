#include "Renderer.h"

#include "Animation.h"
#include "Environment.h"
#include "Factory.h"

#include "nclgl/Camera.h"
#include "nclgl/HeightMap.h"
#include "nclgl/MeshMaterial.h"
#include "nclgl/SceneNode.h"

#define MODELSDIR "../Models/"
#define ANIMATIONDIR "../Animations/"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	defaultProjMatrix = Matrix4::Perspective(0.1f, 10000.0f, (float)width / (float)height, 45.0f);

	// Position treated as direction
	sun = new DirectionalLight(Vector3(0.2f, -1.0f, -0.3f), Vector4(0.7f, 0.7f, 0.75f, 1));
	sunShader = new Shader("combineVert.glsl", "DirectionalLightFrag.glsl");

	unsigned int flags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS;

	// Construction yard skybox
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"/CloudySky/px.png", TEXTUREDIR"/CloudySky/nx.png",
		TEXTUREDIR"/CloudySky/py.png", TEXTUREDIR"/CloudySky/ny.png",
		TEXTUREDIR"/CloudySky/pz.png", TEXTUREDIR"/CloudySky/nz.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	quad = Mesh::GenerateQuad();
	skyboxShader = new Shader("SkyboxVertex.glsl", "SkyboxFragment.glsl");
	if (!skyboxShader->LoadSuccess()) return;

	// Concrete ground
	MeshMaterial material("ConcreteFloor.mat"); // for a single mesh (floor)
	const MeshMaterialEntry* matEntry = material.GetMaterialForLayer(0);

	// Max supported anisotropy level of gpu
	GLfloat maxAniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);

	for (const auto& [type, filename] : matEntry->entries) {
		std::string path = TEXTUREDIR + filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, flags);
		if (!texID) return;

		// Apply Anisotropic Filtering to fix "dithering" when camera is low
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
		glBindTexture(GL_TEXTURE_2D, 0);

		concreteTextures[type] = texID;
	}

	// Scale 1 unit = 0.1m
	// Tex size: 1024 x 1024
	const Vector3 vertexScale = Vector3(0.2f, 1,0.2f); // roughly 200 x 200
	const Vector2 textureScale = Vector2(30, 30); // repeats every 8 m
	concreteMap = new HeightMap(concreteTextures["Displacement"], vertexScale, textureScale);

	concreteShader = new Shader(
		"ConcreteFloorVert.glsl", 
		"ConcreteFloorFrag.glsl",
		"", 
		"ConcreteFloorTCS.glsl",
		"ConcreteFloorTES.glsl"); // No geometry shader
	if (!concreteShader->LoadSuccess()) return;
	Vector3 dimensions = concreteMap->GetHeightmapSize();

	environmentShader = new Shader("BumpVertex.glsl", "PBRFrag.glsl");
	if (!environmentShader->LoadSuccess()) return;

	// Load city scene
	factory = new Factory(MESHDIR"/Factory/scene.gltf"); 

	Matrix4 cityTransformation = Matrix4::Translation(dimensions * Vector3(0.65f, 0, 0.2f));
	factory->SetTransform(cityTransformation);
	//factory->SetModelScale(Vector3(35));

	// Load robot with walk anim
	robot = new RobotModel(MESHDIR"Robot.fbx");
	robot->SetModelScale(Vector3(1 / 60.0f));
	robot->SetTransform(Matrix4::Translation(dimensions * Vector3(0.5f, 0, 0.5f)));

	Animation* anim = new Animation(MODELSDIR"Robot/RobotModelWithWalkAnim.fbx", robot);
	animator = new Animator(anim);
	robotShader = new Shader("SkinningVertex.glsl", "PBRFrag.glsl");

	// Load lamp post
	// From debugging
	/*	Vector3(137.997, 0, 84.7741)
		Vector3(137.997, 0, 67.2194)
		Vector3(114.998, 0, 67.2194)	*/
	lampPost = new LampPost(MESHDIR"LampPost/Street_light.obj");
	lampPost->SetModelScale(Vector3(1.5));
	lampPost->SetTransform(
		Matrix4::Translation(Vector3(140, 0, 80)) * Matrix4::Rotation(180, Vector3(0, 1, 0)));
	
	// Point light and combine shaders
	pointLightShader = new Shader("pointLightVert.glsl", "pointLightFrag.glsl");
	combineShader = new Shader("combineVert.glsl", "combineFrag.glsl");
	if (!pointLightShader->LoadSuccess() || !combineShader->LoadSuccess()) return;
	lightVolume = Mesh::LoadFromMeshFile("Sphere.msh");
	
	camera = new Camera(-3, 0, dimensions * Vector3(0.525, 0.1, 0.5), 20);

	// FBOs
	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &pointLightFBO);

	GLenum buffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2}; // Colour, Normal, Emissive textures
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
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	init = true;
}

Renderer::~Renderer(void) {
	delete camera;

	delete robotShader;
	delete robot;
	delete lampPost;

	delete animator;

	delete skyboxShader;
	delete quad;

	delete environmentShader;
	delete factory;
	delete lightVolume;

	delete pointLightShader;
	delete combineShader;

	delete concreteShader;
	delete concreteMap;

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

	//DrawRobotEmissive();
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();

	factory->Update(dt);
	lampPost->Update(dt);

	robot->Update(dt);
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

	
	projMatrix = defaultProjMatrix;

	DrawConcreteFloor();

	DrawFactory();
	DrawLampPost();

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
	//DrawPointLights(invViewProj, camPos);

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

	Matrix4 cameraView = camera->BuildViewMatrix();
	Matrix4 invProjView = (defaultProjMatrix * cameraView).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(combineShader->GetProgram(), "inverseProjView"), 1, false, invProjView.values);

	auto camPos = camera->GetPosition();
	glUniform3fv(glGetUniformLocation(combineShader->GetProgram(), "cameraPos"), 1, (float*)&camPos);

	quad->Draw();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	BindShader(skyboxShader);

	projMatrix = defaultProjMatrix;

	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawConcreteFloor() {
	BindShader(concreteShader);
	GLuint programLocation = concreteShader->GetProgram();

	glPatchParameteri(GL_PATCH_VERTICES, 3);

	auto BindTex = [&](const char* name, int slot, GLuint texID) {
		glUniform1i(glGetUniformLocation(programLocation, name), slot);
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, texID);
		};

	BindTex("diffuseTex",		0,	concreteTextures["Diffuse"]);
	BindTex("bumpTex",			1,	concreteTextures["Bump"]);
	BindTex("aoTex",			2,	concreteTextures["AO"]);
	BindTex("displacementTex",	3,	concreteMap->GetTexture());
	BindTex("roughnessTex",		4,	concreteTextures["Roughness"]);
	BindTex("noiseTex",			5,	concreteTextures["Noise"]);

	// Tesselation level
	// Higher = smoother geometry, more expensive
	glUniform1f(glGetUniformLocation(programLocation, "tessLevel"), 32.0f);

	// Displacement Strength
	// Height of bumps in world units
	glUniform1f(glGetUniformLocation(programLocation, "displacementStrength"), 0.04);

	glUniform1i(glGetUniformLocation(programLocation, "texWidth"), concreteMap->GetTexWidth());

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	//glDisable(GL_CULL_FACE);
	concreteMap->Draw();
	//glEnable(GL_CULL_FACE);
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

void Renderer::DrawFactory() {
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

void Renderer::DrawLampPost() {
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

	DrawNode(lampPost);
}

void Renderer::DrawRobot() {
	BindShader(robotShader);
	GLuint programLocation = robotShader->GetProgram();

	auto& transforms = animator->GetFinalBoneMatrices();
	glUniformMatrix4fv(glGetUniformLocation(programLocation, "joints"), transforms.size(), false, (float*)transforms.data());

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

	DrawNode(robot);
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
