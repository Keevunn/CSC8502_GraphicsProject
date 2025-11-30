#pragma once
#include <vector>
#include "Matrix4.h"
#include "Mesh.h"
#include "Vector3.h"
#include "Vector4.h"

struct MaterialTextures {
	GLuint diffuseID = 0;
	GLuint bumpID = 0;
	GLuint alphaID = 0;
	GLuint roughnessID = 0;
	GLuint metallicID = 0;
	GLuint emissiveID = 0;
};

class SceneNode {
public:
	SceneNode(Mesh* mesh = nullptr, const Vector4& colour = Vector4(1, 1, 1, 1))
		: mesh(mesh), modelScale(Vector3(1, 1, 1)), colour(colour), distanceFromCamera(0), boundingRadius(1) {}
	SceneNode(const SceneNode& other);

	virtual ~SceneNode(void);

	float GetBoundingRadius() const { return boundingRadius; }
	void SetBoundingRadius(float f) { boundingRadius = f; }

	float GetCameraDistance() const { return distanceFromCamera; }
	void SetCameraDistance(float f) { distanceFromCamera = f; }

	MaterialTextures GetMaterial() const { return material; }
	void SetMaterial(MaterialTextures mat) { material = mat; }

	static bool CompareByCameraDistance(SceneNode* a, SceneNode* b) { return a->distanceFromCamera < b->distanceFromCamera; }

    void SetTransform(const Matrix4& matrix) { transform = matrix; }
    const Matrix4& GetTransform() const { return transform; }
    Matrix4 GetWorldTransform() const { return worldTransform; }

	Vector4 GetColour() const { return colour; }
	void SetColour(const Vector4& c) { colour = c; }

    Vector3 GetModelScale() const { return modelScale; }
	virtual void SetModelScale(const Vector3& s) { modelScale = s; }

    Mesh* GetMesh() const { return mesh; }
	void SetMesh(Mesh* m) { mesh = m; }

	std::string GetName() const { return name; }
	void SetName(const std::string n) { name = n; }

	SceneNode* GetParent() const { return parent; }
    void AddChild(SceneNode* s);

	virtual void Update(float dt);
    virtual void Draw(const OGLRenderer& r);

	std::vector<SceneNode*>::const_iterator GetChildIteratorStart() const { return children.begin(); }
	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd() const { return children.end(); }

 protected:
	SceneNode* parent = nullptr;
 	Mesh* mesh = nullptr;
	Matrix4 worldTransform;
	Matrix4 transform;
	Vector3 modelScale;
	Vector4 colour;
	std::vector<SceneNode*> children;

	float distanceFromCamera;
	float boundingRadius;

	MaterialTextures material;

	std::string name; // For debugging
};

