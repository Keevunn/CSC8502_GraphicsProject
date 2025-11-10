#include "HeightMap.h"
#include <iostream>

HeightMap::HeightMap(const std::string& filename) {
	int iWidth, iHeight, iChans;
	unsigned char* data = SOIL_load_image(filename.c_str(), &iWidth, &iHeight, &iChans, 1);
	if (!data) {
		std::cout << "Heightmap can't load file\n";
		return;
	}

	numVertices = iWidth * iHeight;
	numIndices = (iWidth - 1) * (iHeight - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];

	Vector3 vertexScale = Vector3(16, 1, 16);
	Vector2 textureScale = Vector2(1 / 16.0f, 1 / 16.0f);

	for (int z{}; z < iHeight; ++z) {
		for (int x{}; x < iWidth; ++x) {
			int offset = (z * iWidth) + x;
			vertices[offset] = Vector3(static_cast<float>(x), data[offset], static_cast<float>(z)) * vertexScale;
			textureCoords[offset] = Vector2(static_cast<float>(x), static_cast<float>(z)) * textureScale;
		}
	}
	SOIL_free_image_data(data);

	int i = 0;
	for (int z{}; z < iHeight - 1; ++z) {
		for (int x{}; x < iWidth - 1; ++x) {
			int a = (z * iWidth) + x; // TopLeft	
			int b = (z * iWidth) + (x + 1); // TopRight
			int c = ((z + 1) * iWidth) + (x + 1); // BottomRight
			int d = ((z + 1) * iWidth) + x; // BottomLeft

			indices[i++] = a; // Using anticlockwise winding
			indices[i++] = c;
			indices[i++] = b;

			indices[i++] = c;
			indices[i++] = a;
			indices[i++] = d;
		}
	}
	GenerateNormals();
	BufferData();

	heightmapSize.x = vertexScale.x * (iWidth - 1);
	heightmapSize.y = 255.0f * vertexScale.y; // each height is a byte
	heightmapSize.z = vertexScale.z * (iHeight - 1);
}
