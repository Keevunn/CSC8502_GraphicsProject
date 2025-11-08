#pragma once
#include "nclgl/OGLRenderer.h"
#include "nclgl/HeightMap.h"
#include "nclgl/Camera.h"

class Renderer : public OGLRenderer{
public:
	Renderer(Window &parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void PresentScene();
	void DrawPostProcess();
	void DrawScene();

	Shader* sceneShader;
	Shader* postProcessShader;

	Camera* camera;

	Mesh*		quad;
	HeightMap*	heightMap;
	GLuint		heightTexture;
	GLuint		bufferFBO;
	GLuint		postProcessFBO;
	GLuint		bufferColourTex[2];
	GLuint		bufferDepthTex;
};

