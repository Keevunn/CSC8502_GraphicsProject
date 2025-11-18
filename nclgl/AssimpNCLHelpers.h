#pragma once
#include <assimp/matrix4x4.h>
#include <assimp/vector2.h>
#include <assimp/vector3.h>

#include "Matrix4.h"
#include "Vector2.h"
#include "Vector3.h"

class AssimpNCLHelpers {
public:

	static Matrix4 GetNCLMatrix(const aiMatrix4x4& from) {
		Matrix4 to;

		// Assimp rows - a, b, c, d
		// Assimp columns - 1, 2, 3, 4
		to.values[0] = from.a1; to.values[1] = from.a2; to.values[2] = from.a3; to.values[3] = from.a4;
		to.values[4] = from.b1; to.values[5] = from.b2; to.values[6] = from.b3; to.values[7] = from.b4;
		to.values[8] = from.c1; to.values[9] = from.c2; to.values[10] = from.c3; to.values[11] = from.c4;
		to.values[12] = from.d1; to.values[13] = from.d2; to.values[14] = from.d3; to.values[15] = from.d4;

		return to;
	}

	static Vector3 GetNCLVec(const aiVector3D& vec) { return Vector3{vec.x, vec.y, vec.z}; }
	static Vector2 GetNCLVec(const aiVector2D& vec) { return Vector2{ vec.x, vec.y }; }
};

