#include "HeightMap.h"
#include <iostream>

HeightMap::HeightMap(const std::string& filename, const Vector3& vertexScale, const Vector2& textureScale) {
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
	GenerateTangents();

	BufferData();

	heightmapSize.x = vertexScale.x * (iWidth - 1);
	heightmapSize.y = 255.0f * std::max(vertexScale.y, 1.0f); // each height is a byte
	heightmapSize.z = vertexScale.z * (iHeight - 1);
}

// Displacement Map Constructor
// Pass in the displacement map
HeightMap::HeightMap(const GLuint texID, const Vector3& vertexScale, const Vector2& textureScale) {
	texture = texID;

	// Get dimensions
	glBindTexture(GL_TEXTURE_2D, texture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);

	// Set Clamping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Define Mesh Res (low poly for tesselation)
	int gridW = 64; int gridH = 64;

	numVertices = gridW * gridH;
	numIndices = (gridW - 1) * (gridH - 1) * 6;

	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];

	normals = new Vector3[numVertices];
	tangents = new Vector4[numVertices];

	float worldSizeX = (texWidth - 1) * vertexScale.x;
	float worldSizeZ = (texHeight - 1) * vertexScale.z;

	float stepX = worldSizeX / (gridW - 1);
	float stepZ = worldSizeZ / (gridH - 1);

	for (int z = 0; z < gridH; ++z) {
		for (int x = 0; x < gridW; ++x) {
			int offset = (z * gridW) + x;

			// Flat position (y=0). Displacement happens in the Shader.
			vertices[offset] = Vector3(x * stepX, 0.0f, z * stepZ);

			// Texture map 0.0-1.0 across the whole grid
			textureCoords[offset] = Vector2((float)x / (gridW - 1),(float)z / (gridH - 1)) * textureScale;

			// Default normals and tangents
			normals[offset] = Vector3(0, 1, 0);
			tangents[offset] = Vector4(1, 0, 0, 1);
		}
	}

	int i = 0;
	for (int z{}; z < gridH - 1; ++z) {
		for (int x{}; x < gridW - 1; ++x) {
			int a = (z * gridW) + x; // TopLeft	
			int b = (z * gridW) + (x + 1); // TopRight
			int c = ((z + 1) * gridW) + (x + 1); // BottomRight
			int d = ((z + 1) * gridW) + x; // BottomLeft

			indices[i++] = a; // Using anticlockwise winding
			indices[i++] = c;
			indices[i++] = b;

			indices[i++] = c;
			indices[i++] = a;
			indices[i++] = d;
		}
	}

	// Normals calculated by TCS
	
	BufferData();

	heightmapSize.x = worldSizeX;
	heightmapSize.y = 255.0f * std::max(vertexScale.y, 1.0f);
	heightmapSize.z = worldSizeZ;

	type = GL_PATCHES; // For Displacement Map Rendering
}

