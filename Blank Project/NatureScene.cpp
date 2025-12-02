#include "NatureScene.h"

#include "KittenModel.h"
#include "Renderer.h"
#include "nclgl/Camera.h"
#include "nclgl/DirectionalLight.h"
#include "nclgl/HeightMap.h"
#include "nclgl/MeshMaterial.h"

NatureScene::~NatureScene() {
	delete terrain;
	delete waterHeightMap;

	if (terrainData) SOIL_free_image_data(terrainData);
}

void NatureScene::Initialise(Renderer& renderer) {
	sun = new DirectionalLight(Vector3(0.2f, -1.0f, -0.3f), Vector4(1, 0.95f, 0.8f, 1.0f), Vector4(1, 1, 1, 1));
	fogDensity = 0.45f;
	float aspect = static_cast<float>(renderer.GetWidth()) / static_cast<float>(renderer.GetHeight());
	defaultProjMatrix = Matrix4::Perspective(0.1f, 10000.0f, aspect, 45.0f);

	// Camera settings
	camera = new Camera(0, 0, Vector3(100, 27, 140), 10);

	cameraPath = {	
		CameraWaypoint(Vector3(101.028f,30.1109f,187.031f), -19.6f, 3.29018f),						// View down -Z
		CameraWaypoint(Vector3(156.243f,14.0005f,103.488f), -16.4501f, 114.592f, 8),	// Over dancing cats
		CameraWaypoint(Vector3(183.196f,21.7224f,80.6437f), -12.8102f, 101.221f),					// View down +X
		CameraWaypoint(Vector3(136.303f,13.8901f,58.7426f), -18.9002f, 98.3512f, 8),	// By sitting and swimming Cats
		CameraWaypoint(Vector3(124.369f,20.3891f,23.6196f), -7.70014f, 167.581f),					// View down +Z
		CameraWaypoint(Vector3(51.6245f,15.3903f,40.073f), -0.840291f, 112.911f, 4),	// Mountain cat view 1
		CameraWaypoint(Vector3(21.7816f,20.9466f,60.8336f), -21.4203f, 307.712f, 6),	// Mountain cat view 2
		CameraWaypoint(Vector3(42.0774f,25.2797f,125.497f), -18.3406f, 291.826f),					// View down -X
	};

	unsigned int flags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS;

	skybox = SOIL_load_OGL_cubemap(
		TEXTUREDIR"/BrightSky/px.png", TEXTUREDIR"/BrightSky/nx.png",
		TEXTUREDIR"/BrightSky/py.png", TEXTUREDIR"/BrightSky/ny.png",
		TEXTUREDIR"/BrightSky/pz.png", TEXTUREDIR"/BrightSky/nz.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	if (!skybox) return;

	GLuint terrainHeightMap = SOIL_load_OGL_texture(TEXTUREDIR"NatureSceneHeightMap.png", SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, flags);
	if (!terrainHeightMap) return;

	const Vector3 vertexScale = Vector3(0.2f, 1/5.0f, 0.2f); // vertex scale y is ignored for tessellation
	const Vector2 textureScale = Vector2(1, 1);
	terrain = new HeightMap(terrainHeightMap, vertexScale, textureScale); 

	auto LoadMaterialTextures = [&](const MeshMaterialEntry* matEntry, std::unordered_map<std::string, GLuint>& textureList) {
			for (const auto& [type, filename] : matEntry->entries) {
				std::string path = TEXTUREDIR + filename;
				GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, flags);
				if (!texID) return;
				textureList[type] = texID;
			}
		};
	MeshMaterial materials("NatureSceneBlendMap.mat");
	LoadMaterialTextures(materials.GetMaterialForLayer(0), baseTextures);
	LoadMaterialTextures(materials.GetMaterialForLayer(1), sandTextures);
	LoadMaterialTextures(materials.GetMaterialForLayer(2), grassTextures);
	LoadMaterialTextures(materials.GetMaterialForLayer(3), rockyTextures);

	terrainBlendMap = SOIL_load_OGL_texture(TEXTUREDIR"NatureSceneSplatMap.png", SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, flags);
	if (!terrainBlendMap) return;

	waterMask = SOIL_load_OGL_texture(TEXTUREDIR"WaterMask.png", SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, flags);
	GLuint flatMap = SOIL_load_OGL_texture(TEXTUREDIR"FlatHeightMap.png", SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, flags);
	if (!waterMask || !flatMap) return;
	waterHeightMap = new HeightMap(flatMap, vertexScale, textureScale);

	// Kittens
	int channels, mapWidth, mapHeight;
	terrainData = SOIL_load_image(TEXTUREDIR"NatureSceneHeightMap.png", &mapWidth, &mapHeight, &channels, SOIL_LOAD_L);

	const std::shared_ptr<KittenModel> globalKittenModel = renderer.GetGlobalKittenModel();
	Matrix4 initialTransform = globalKittenModel->GetTransform();
	Vector3 position;
	
	// Dancing Cats
	KittenModel* dancingCat1 = new KittenModel(*globalKittenModel);
	dancingCat1->PlayAnimation("Dancing1");
	kittens.push_back(dancingCat1);

	position = Vector3(140.2f, 0, 107.2f);
	position.y = GetTerrainHeight(position.x, position.z, mapWidth, mapHeight);
	dancingCat1->SetTransform(Matrix4::Translation(position) * Matrix4::Rotation(50, Vector3(0, 1, 0))  * initialTransform);

	KittenModel* dancingCat2 = new KittenModel(*globalKittenModel);
	dancingCat2->PlayAnimation("Dancing2");
	kittens.push_back(dancingCat2);

	position = Vector3(146.2f, 0, 110.2f);
	position.y = GetTerrainHeight(position.x, position.z, mapWidth, mapHeight);
	dancingCat2->SetTransform(Matrix4::Translation(position) * Matrix4::Rotation(-100, Vector3(0, 1, 0)) * initialTransform);

	// Sitting Cat and Swimming Cat
	KittenModel* sittingPose = new KittenModel(*globalKittenModel);
	sittingPose->PlayAnimation("SittingPose");
	kittens.push_back(sittingPose);
	
	position = Vector3(124.6f, 0.25, 54.8f);
	position.y += GetTerrainHeight(position.x, position.z, mapWidth, mapHeight);
	sittingPose->SetTransform(Matrix4::Translation(position) * Matrix4::Rotation(-30, Vector3(0, 1, 0))  * initialTransform);

	KittenModel* treadingWater = new KittenModel(*globalKittenModel);
	treadingWater->PlayAnimation("TreadingWater");
	kittens.push_back(treadingWater);
	treadingWater->SetTransform(Matrix4::Translation(Vector3(115.2f, 3.75f, 66.8f)) * Matrix4::Rotation(135, Vector3(0, 1, 0)) * initialTransform); // Relative to water offset 

	// Mountain Cat
	KittenModel* swingingLegs = new KittenModel(*globalKittenModel);
	swingingLegs->PlayAnimation("SwingingLegs");
	kittens.push_back(swingingLegs);

	position = Vector3(36.2f, -0.25f, 42.4f);
	position.y += GetTerrainHeight(position.x, position.z, mapWidth, mapHeight);
	swingingLegs->SetTransform(Matrix4::Translation(position) * Matrix4::Rotation(45, Vector3(0, 1, 0)) * initialTransform);

	init = true;
}

void NatureScene::Update(float dt) {
	Scene::Update(dt);

	for (auto* kitten : kittens)
		kitten->Update(dt);

}

void NatureScene::RenderGeometry(Renderer& renderer) {
	renderer.DrawTerrain(terrain, terrainBlendMap, baseTextures, sandTextures, grassTextures, rockyTextures);
	renderer.DrawWater(waterMask, skybox, waterHeightMap);
	renderer.DrawKittens(kittens);
}

float NatureScene::GetTerrainHeight(float x, float z, int mapWidth, int mapHeight) {
	if (!terrainData) return 0.0f;

	// Must match vertex scale y
	float terrainXSize = (mapWidth - 1) * 0.2f;
	float terrainZSize = (mapHeight - 1) * 0.2f;
	if (x < 0 || z < 0 || x > terrainXSize || z > terrainZSize) return 0.0f;

	int pixelX = (x / terrainXSize) * (mapWidth - 1);
	int pixelZ = (z / terrainZSize) * (mapHeight - 1);

	int flippedZ = (mapHeight - 1) - pixelZ;

	unsigned char pixelHeight = terrainData[(flippedZ * mapWidth) + pixelX];

	float maxTerrainHeight = 25.0f; // Hardcoded to match displacementStrength
	return (pixelHeight / 255.0f) * maxTerrainHeight;
}
