#pragma once
#include "../nclgl/OGLRenderer.h"

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	virtual ~Renderer(void) override;

	virtual void RenderScene() override;

	void SwitchToPerspective();
	void SwitchToOrthographic();

	inline void SetScale(float s) { scale = s; }
	inline void SetRotation(float r) { rotation = r; }
	inline void SetPosition(Vector3 p) { position = p; }
	inline void SetFOV(float f) { fov = f; } // fov is a value between 0 and 1

protected:
	Mesh* triangle;
	Shader* matrixShader;
	float scale;
	float rotation;
	Vector3 position;
	float fov;
	bool inPerspective = false;
};