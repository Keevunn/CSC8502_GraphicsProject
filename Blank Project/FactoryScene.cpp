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

void FactoryScene::Initialise(float width, float height) {
	sun = new DirectionalLight(Vector3(0.2f, -1.0f, -0.3f), Vector4(0.3f, 0.35f, 0.2f, 1), Vector4(0.4f, 0.4f, 0.2f, 1));
	defaultProjMatrix = Matrix4::Perspective(0.1f, 10000.0f, width / height, 30.0f);
	camera = new Camera(-10, 0, Vector3(133, 5, 165), 10);

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
	RobotModel sharedRobotMesh = RobotModel(MODELSDIR"Robot/RobotModelWithWalkAnim.fbx");
	sharedRobotMesh.SetModelScale(Vector3(1 / 350.0f));
	robots = new AnimatedModelPool(sharedRobotMesh, Vector3(133, 0, 55), 165, Vector3(0, 0, 1));

	KittenModel sharedKittenMesh = KittenModel(MODELSDIR"KittenWithSadWalk.fbx");
	sharedKittenMesh.SetModelScale(Vector3(1 / 100.0f));
	sharedKittenMesh.SetTransform(Matrix4::Rotation(-90, Vector3(0, 1, 0)));
	kittens = new AnimatedModelPool(sharedKittenMesh, Vector3(111, 0, 165), 47, Vector3(0, 0, -1));

}

void FactoryScene::Update(float dt) {
	camera->UpdateCamera(dt);

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
