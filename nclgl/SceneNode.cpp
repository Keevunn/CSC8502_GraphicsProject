#include "SceneNode.h"

SceneNode::~SceneNode(void) {
	for (unsigned int i{}; i < children.size(); ++i)
		delete children[i];
	parent = nullptr;
}

void SceneNode::AddChild(SceneNode* s) {
	s->parent = this;
	children.push_back(s);
}

void SceneNode::Draw(const OGLRenderer& r) {
	if (mesh) mesh->Draw();
}

void SceneNode::Update(float dt) {
	if (parent) worldTransform = parent->worldTransform * transform;
	else worldTransform = transform;

	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); ++i)
		(*i)->Update(dt);
}