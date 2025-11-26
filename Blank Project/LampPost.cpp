#include "LampPost.h"

#include <assimp/postprocess.h>

LampPost::LampPost(const std::string path) {
	auto flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GlobalScale | aiProcess_CalcTangentSpace;
	texDir = TEXTUREDIR"/LampPost/";
	Environment::LoadScene(path, "Lamp_Root", flags);
}
