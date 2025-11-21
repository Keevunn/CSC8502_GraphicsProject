/******************************************************************************
Class:Mesh
Implements:
Author:Rich Davison	 <richard-gordon.davison@newcastle.ac.uk>
Description:Wrapper around OpenGL primitives, geometry and related 
OGL functions.

There's a couple of extra functions in here that you didn't get in the tutorial
series, to draw debug normals and tangents. 


-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "OGLRenderer.h"
#include <vector>
#include <string>
#include <unordered_map>

#include <assimp/mesh.h>
#include <assimp/scene.h>


struct BoneInfo {
	int id; //index in boneMatrices
	Matrix4 invBindPose; //transforms vertex from model space to bone space
};

//A handy enumerator, to determine which member of the bufferObject array
//holds which data
enum MeshBuffer {
	VERTEX_BUFFER	,
	COLOUR_BUFFER	, 
	TEXTURE_BUFFER	,
	NORMAL_BUFFER	, 
	TANGENT_BUFFER	,

	WEIGHTVALUE_BUFFER,		//new this year, weight values of vertices
	WEIGHTINDEX_BUFFER,	//new this year, indices of weights

	INDEX_BUFFER	,

	MAX_BUFFER
};

class Mesh	{
public:	
	struct SubMesh {
		int start;
		int count;
	};

	Mesh(void);
	~Mesh(void);

	void Draw();
	void DrawSubMesh(int i);

	static Mesh* LoadFromMeshFile(const std::string& name);
	static Mesh* LoadFromAssimpMesh(::aiMesh* aiMesh, const aiScene* scene, std::unordered_map<std::string, BoneInfo>& boneInfoMap, int& boneCounter);

	unsigned int GetTriCount() const {
		int primCount = indices ? numIndices : numVertices;
		return primCount / 3;
	}

	unsigned int GetJointCount() const {
		return (unsigned int)jointNames.size();
	}


	int GetIndexForJoint(const std::string& name) const;
	int GetParentForJoint(const std::string& name) const;
	int GetParentForJoint(int i) const;

	const Matrix4* GetBindPose() const {
		return bindPose;
	}

	const Matrix4* GetInverseBindPose() const {
		return inverseBindPose;
	}

	int		GetSubMeshCount() const {
		return (int)meshLayers.size(); 
	}

	bool GetSubMesh(int i, const SubMesh* s) const;
	bool GetSubMesh(const std::string& name, const SubMesh* s) const;

	static Mesh* GenerateTriangle();
	static Mesh* GenerateQuad();

	void	GenerateNormals();
	bool	GetVertexIndicesForTri(unsigned int i, unsigned int& a, unsigned int& b, unsigned int& c) const;

protected:
	void	BufferData();

	void	GenerateTangents();
	Vector4 GenerateTangent(int a, int b, int c);

	void SetVertexBoneData(unsigned int vertexID, unsigned int boneID, float weight, int slot);
	void GetBoneWeightsForVertices(const aiMesh* aiMesh, std::vector<int>& nextSlot, std::unordered_map<std::string, BoneInfo>& boneInfoMap, int& boneCounter);

	GLuint	arrayObject;

	GLuint	bufferObject[MAX_BUFFER];

	GLuint	numVertices;
	GLuint	numIndices;
	
	GLuint	type;

	Vector3*		vertices;
	Vector4*		colours;
	Vector2*		textureCoords;
	Vector3*		normals;
	Vector4*		tangents;

	Vector4*		weights;
	int*			weightIndices;

	unsigned int*	indices;

	Matrix4* bindPose;
	Matrix4* inverseBindPose;

	std::vector<std::string>	jointNames;
	std::vector<int>			jointParents;
	std::vector< SubMesh>		meshLayers;
	std::vector<std::string>	layerNames;
};

