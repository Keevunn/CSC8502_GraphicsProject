#include "RobotModel.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "nclgl/AssimpNCLHelpers.h"

RobotModel::RobotModel(const std::string& path)  {
	RobotModel::LoadScene(path);
}

void RobotModel::LoadScene(const std::string& path) {
	Assimp::Importer importer;
	//importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	// aiProcess_Triangulate: converts non-triangle faces to triangles
	// aiProcess_GenSmoothNormals: Creates normals if missing
	// aiProcess_CalcTangentSpace: Calculates tangents and bitangents
	const aiScene* scene = importer.ReadFile(path, 
		aiProcess_Triangulate | 
		aiProcess_GenSmoothNormals | 
		aiProcess_CalcTangentSpace | 
		aiProcess_SortByPType | 
		aiProcess_JoinIdenticalVertices | 
		aiProcess_LimitBoneWeights |
		aiProcess_PopulateArmatureData);
	
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "ERROR: ASSIMP: " << importer.GetErrorString() << std::endl;
		return;
	}
	name = "Robot_Root";
	//dir = path.substr(0, path.find_last_of('/'));
	LoadMaterials();

	ProcessNode(scene->mRootNode, scene, this);
}

void RobotModel::LoadMaterials() {
	std::string texDir = TEXTUREDIR"Robot/";
	MaterialTextures mat;
	unsigned int flags = SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS;

	auto LoadTex = [&](std::string filename, bool useRGB = false) {
		const std::string path = texDir + filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), useRGB ? SOIL_LOAD_RGB : SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, flags);
		if (!texID) std::cout << "Failed texture: " << filename << "\n"
							<< SOIL_last_result() << "\n";
		return texID;
		};

	mat.diffuseID = LoadTex("Robot_Base_color 5.png");
	mat.bumpID = LoadTex("Robot_Normal_OpenGL_fixed.png");
	mat.alphaID = LoadTex("Robot_Opacity.png");
	mat.specularID = LoadTex("Robot_Roughness.png");
	mat.metallicID = LoadTex("Robot_Metallic.png");
	mat.emissiveID = LoadTex("Robot_Emissive.png");

	materials.push_back(mat);
}

Mesh* RobotModel::LoadMesh(const aiMesh* aiMesh, const aiScene* scene) {
	return Mesh::LoadFromAssimpMesh(aiMesh, scene, boneInfoMap, boneCounter);
}
