#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include <vector>

class SceneNode {
public:
	SceneNode(Mesh* mesh = nullptr, const Vector4& colour = Vector4(1, 1, 1, 1))
		: parent(nullptr), mesh(mesh), modelScale(Vector3(1, 1, 1)), colour(colour), distanceFromCamera(0), boundingRadius(1), texture(0) {}

	virtual ~SceneNode(void);

	float GetBoundingRadius() const { return boundingRadius; }
	void SetBoundingRadius(float f) { boundingRadius = f; }

	float GetCameraDistance() const { return distanceFromCamera; }
	void SetCameraDistance(float f) { distanceFromCamera = f; }

	GLuint GetTexture() const { return texture; }
	void SetTexture(GLuint tex) { texture = tex; }

	static bool CompareByCameraDistance(SceneNode* a, SceneNode* b) { return a->distanceFromCamera < b->distanceFromCamera; }

    void SetTransform(const Matrix4& matrix) { transform = matrix; }
    const Matrix4& GetTransform() const { return transform; }
    Matrix4 GetWorldTransform() const { return worldTransform; }

	Vector4 GetColour() const { return colour; }
	void SetColour(const Vector4& c) { colour = c; }

    Vector3 GetModelScale() const { return modelScale; }
	void SetModelScale(const Vector3& s) { modelScale = s; }

    Mesh* GetMesh() const { return mesh; }
	void SetMesh(Mesh* m) { mesh = m; }

	std::string GetName() const { return name; }
	void SetName(const std::string n) { name = n; }

    void AddChild(SceneNode* s);

	virtual void Update(float dt);
    virtual void Draw(const OGLRenderer& r);

	std::vector<SceneNode*>::const_iterator GetChildIteratorStart() const { return children.begin(); }
	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd() const { return children.end(); }

 protected:
	SceneNode* parent;
 	Mesh* mesh;
	Matrix4 worldTransform;
	Matrix4 transform;
	Vector3 modelScale;
	Vector4 colour;
	std::vector<SceneNode*> children;

	float distanceFromCamera;
	float boundingRadius;
	GLuint texture;

	std::string name = ""; // For debugging
};

