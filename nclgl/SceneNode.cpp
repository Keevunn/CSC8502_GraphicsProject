#include "SceneNode.h"

SceneNode::~SceneNode(void) {
	for (unsigned int i{}; i < children.size(); ++i)
		delete children[i];
}

void SceneNode::AddChild(SceneNode* s) {
	s->parent = this;
	children.push_back(s);
}

void SceneNode::Draw(const OGLRenderer& r) {
	if (!mesh) return;

	auto BindTex = [&](int slot, GLuint id) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, id);
		};

	BindTex(0, material.diffuseID);
	BindTex(1, material.bumpID);
	BindTex(2, material.reflectionID);
	BindTex(3, material.alphaID);
	BindTex(4, material.specularID);
	BindTex(5, material.metallicID);
	BindTex(6, material.emissiveID);

	mesh->Draw();
}

void SceneNode::Update(float dt) {
	if (parent) worldTransform = parent->worldTransform * transform;
	else worldTransform = transform;

	worldTransform = worldTransform * Matrix4::Scale(modelScale);

	for (auto& childNode : children)
		childNode->Update(dt);
}