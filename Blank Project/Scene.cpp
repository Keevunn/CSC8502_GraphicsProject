#include "Scene.h"

#include "nclgl/Camera.h"
#include "nclgl/DirectionalLight.h"
#include "nclgl/SceneNode.h"

Scene::~Scene() {
	delete camera; 
	delete sun;
}

