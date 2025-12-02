#pragma once
#include "Mesh.h"
#include <string>

class HeightMap : public Mesh {
public:
	HeightMap(const std::string& filename, const Vector3& vertexScale = Vector3(16.0f, 1.0f, 16.0f), const Vector2& textureScale = Vector2(1/16.0f, 1/16.0f));
	HeightMap(const GLuint texID, const Vector3& vertexScale = Vector3(16.0f, 1.0f, 16.0f), const Vector2& textureScale = Vector2(1/16.0f, 1/16.0f));
	~HeightMap() = default;

	Vector3 GetHeightmapSize() const { return heightmapSize; }
	
	GLuint GetTexture() const { return texture; }
	void SetTexture(GLuint tex) { texture = tex; }

	int GetTexWidth() const { return texWidth; }
	int GetTexHeight() const { return texHeight; }

protected:
	Vector3 heightmapSize;
	GLuint texture;
	int texWidth, texHeight;
};

