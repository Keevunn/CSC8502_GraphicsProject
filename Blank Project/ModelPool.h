#pragma once
#include <vector>

#include "Environment.h"
#include "nclgl/Vector3.h"
#include "nclgl/Vector2.h"

class AnimatedModel;

template <typename T>

class ModelPool {
public:
	
	ModelPool(T sharedMesh, const Vector3& spawnPoint) : sharedMesh(std::move(sharedMesh)), instanceCount(0), spawnPoint(spawnPoint) {}
	// Handles translations
	ModelPool(T sharedMesh, const Vector3& spawnPoint, int instanceCount, float separationDistance = 20.0f) : sharedMesh(sharedMesh), instanceCount(instanceCount), spawnPoint(spawnPoint) {
		static_assert(std::is_base_of_v<Environment, T>, "T must derive from Environment");

		Vector3 startPos = spawnPoint;
		for (int i{}; i < instanceCount; ++i) {
			T* model = new T(this->sharedMesh);
			startPos.z = spawnPoint.z + i * separationDistance;
			Matrix4 currentTransform = Matrix4::Translation(startPos) * model->GetTransform();
			model->SetTransform(currentTransform);
			pool.push_back(model);
		}
	}
	// Handles translations
	ModelPool(const T& sharedMesh, const Vector3& spawnPoint, Vector2 instanceCount, float width, float separationDistance = 20.0f, bool shouldFlip = false) : sharedMesh(sharedMesh), instanceCount(static_cast<int>(instanceCount.x* instanceCount.y)), spawnPoint(spawnPoint) {
		static_assert(std::is_base_of_v<Environment, T>, "T must derive from Environment");

		Vector3 startPos = spawnPoint;
		for (int row{}; row < instanceCount.x; ++row) {
			startPos.z = spawnPoint.z + row * separationDistance;
			for (int col{}; col < instanceCount.y; ++col) {
				T* model = new T(this->sharedMesh);
				startPos.x = spawnPoint.x + col * width;
				Matrix4 rotTransform = shouldFlip ? Matrix4::Rotation(180.0f * col, Vector3(0, 1, 0)) : Matrix4();
				Matrix4 currentTransform = Matrix4::Translation(startPos) * rotTransform * model->GetTransform();
				model->SetTransform(currentTransform);
				pool.push_back(model);
			}
		}
	}
	
	virtual ~ModelPool() {
		for (const T* model : pool)
			delete model;
	}

	virtual void Update(float dt) {
		for (const auto& model : pool)
			model->Update(dt);
	}

	std::vector<T*> GetPool() const { return pool; }
	
protected:
	T sharedMesh;
	int instanceCount;
	Vector3 spawnPoint;

	std::vector<T*> pool;
};

