#include "Factory.h"

#include <assimp/postprocess.h>

Factory::Factory(const std::string path) {
	auto flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GlobalScale | aiProcess_CalcTangentSpace;
	texDir = TEXTUREDIR"/Factory/";
	Environment::LoadScene(path, "Factory_Root", flags);
}
