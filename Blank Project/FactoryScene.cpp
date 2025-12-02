#include "FactoryScene.h"

#include "Factory.h"
#include "KittenModel.h"
#include "LampPost.h"
#include "Renderer.h"
#include "RobotModel.h"
#include "nclgl/Camera.h"
#include "nclgl/DirectionalLight.h"
#include "nclgl/HeightMap.h"
#include "nclgl/MeshMaterial.h"

FactoryScene::~FactoryScene() {
	delete concreteMap;
	delete factory;
	delete robots;
	delete kittens;
}

void FactoryScene::Initialise(Renderer& renderer) {
	sun = new DirectionalLight(Vector3(0.2f, -1.0f, -0.3f), Vector4(0.3f, 0.35f, 0.2f, 1), Vector4(0.4f, 0.4f, 0.2f, 1));
	fogDensity = 1.0f;
	float aspect = static_cast<float>(renderer.GetWidth()) / static_cast<float>(renderer.GetHeight());
	defaultProjMatrix = Matrix4::Perspective(0.1f, 10000.0f, aspect, 30.0f);

	camera = new Camera(-10, 0, Vector3(133, 5, 165), 5);

	cameraPath = {
		CameraWaypoint(Vector3(116.783f,0.833334f,150.984f), 8.41f, 144.9f, 8),
		CameraWaypoint(Vector3(120.159f,5.16667f,107.662f), -9.65001f, 150.99f, 6),
		CameraWaypoint(Vector3(118.779f,2.16667f,48.5625f), -3.84001f, 155.4f),
		CameraWaypoint(Vector3(124.472f,1.66667f,57.2041f), 8.12999f, 49.1402f,4),
		CameraWaypoint(Vector3(115.33f,21.8333f,79.3439f), -17.42f, 13.3002f),
		CameraWaypoint(Vector3(77.9001f,21.8333f,65.8089f), -14.97f, 310.16f, 4),
		CameraWaypoint(Vector3(118.012f,13.8333f,63.8634f), -23.09f, 321.64f, 4),
		CameraWaypoint(Vector3(125.919f,3.16667f,72.7766f), 3.99997f, 341.94f, 7),
		CameraWaypoint(Vector3(127.392f,8.33333f,106.161f), -9.72003f, 344.46f, 8),
		CameraWaypoint(Vector3(138.147f,1.83333f,113.891f), 8.68997f,26.6f, 8),
		CameraWaypoint(Vector3(135.394f,2.33333f,168.181f), -2.65003f, 11.2699f)
	};

	skybox = SOIL_load_OGL_cubemap(
		TEXTUREDIR"/DarkSky/px.png", TEXTUREDIR"/DarkSky/nx.png",
		TEXTUREDIR"/DarkSky/py.png", TEXTUREDIR"/DarkSky/ny.png",
		TEXTUREDIR"/DarkSky/pz.png", TEXTUREDIR"/DarkSky/nz.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	if (!skybox) return;

	// Concrete ground
	MeshMaterial material("ConcreteFloor.mat"); // for a single mesh (floor)
	const MeshMaterialEntry* matEntry = material.GetMaterialForLayer(0);
	unsigned int flags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS;
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
	const Vector3 vertexScale = Vector3(0.2f, 1, 0.2f); // roughly 200 x 200
	const Vector2 textureScale = Vector2(30, 30); 
	concreteMap = new HeightMap(concreteTextures["Displacement"], vertexScale, textureScale);
	Vector3 dimensions = concreteMap->GetHeightmapSize();

	// Load factory
	factory = new Factory(MESHDIR"/Factory/scene.gltf");
	factory->SetTransform(Matrix4::Translation(dimensions * Vector3(0.65f, 0, 0.2f)));
	factory->SetModelScale(Vector3(1.5f));

	// Load lamp posts
	LampPost lampPostMesh = LampPost(MESHDIR"LampPost/Street_light.obj");
	lampPosts = new ModelPool(lampPostMesh, Vector3(107, 0, 60), Vector2(6, 2), 30, 20, true);

	// Load Characters
	RobotModel sharedRobotMesh = RobotModel(MODELSDIR"Robot/RobotModelWithWalkAnim.fbx", "SadWalk");
	sharedRobotMesh.SetModelScale(Vector3(1 / 350.0f));
	robots = new AnimatedModelPool(sharedRobotMesh, Vector3(133, 0, 55), 165, Vector3(0, 0, 1));
	robots->PlayAnimation("SadWalk");

	KittenModel sharedKittenMesh = KittenModel(*renderer.GetGlobalKittenModel());
	Matrix4 initialTransform = sharedKittenMesh.GetTransform();

	sharedKittenMesh.SetMovementVelocity(Vector3(0, 2.5f, 0));
	sharedKittenMesh.SetModelScale(sharedKittenMesh.GetModelScale() * 0.5);
	sharedKittenMesh.SetTransform(Matrix4::Rotation(180, Vector3(0, 1, 0)) * initialTransform);
	kittens = new AnimatedModelPool(sharedKittenMesh, Vector3(111, 0, 165), 47, Vector3(0, 0, -1));
	kittens->PlayAnimation("SadWalk");

	init = true;
}

void FactoryScene::Update(float dt) {
	Scene::Update(dt);

	factory->Update(dt);
	lampPosts->Update(dt);
	robots->Update(dt);
	kittens->Update(dt);
}

void FactoryScene::RenderGeometry(Renderer& renderer) {
	renderer.DrawConcreteFloor(concreteMap, concreteTextures);
	renderer.DrawFactory(factory);
	renderer.DrawLampPosts(lampPosts);
	renderer.DrawRobots(robots);
	renderer.DrawKittens(kittens);
}

void FactoryScene::RenderLights(Renderer& renderer) {
	std::vector<SpotLight*> lights;
	for (const auto* lampPost : lampPosts->GetPool())
		lights.push_back(lampPost->GetLight());
	renderer.DrawSpotLights(lights);
}
