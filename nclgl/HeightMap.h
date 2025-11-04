#pragma once
#include "Mesh.h"
#include <string>

class HeightMap : public Mesh {
public:
	HeightMap(const std::string& filename);
	~HeightMap() = default;

	Vector3 GetHeightmapSize() const { return heightmapSize; }

protected:
	Vector3 heightmapSize;
};

