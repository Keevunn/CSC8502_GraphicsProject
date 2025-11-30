#pragma once
#include "ModelPool.h"

#include <utility>

#include "AnimatedModel.h"


template <typename T>
class AnimatedModelPool : public ModelPool<T> { 
public:
	AnimatedModelPool() = default;
	// Handles translations
	// worldMoveDir - Direction character is moving in world
	// KillPoint - Z coordinate of point that triggers respawn
	AnimatedModelPool(const T& sharedAnimatedMesh, const Vector3& spawnPos, float killPoint, const Vector3& worldMoveDir, float separationDistance = 5.0f) :
	ModelPool<T>(std::move(sharedAnimatedMesh), spawnPos), killPoint(killPoint), worldMoveDir(worldMoveDir) {
		static_assert(std::is_base_of_v<AnimatedModel, T>, "T must derive from AnimatedModel");

		float pathLength = abs(spawnPos.z - killPoint);
		this->instanceCount = static_cast<int>(pathLength / separationDistance);

		Vector3 startPos = this->spawnPoint;
		Vector3 offset = worldMoveDir.Normalised() * separationDistance;
		for (int i{}; i < this->instanceCount; ++i) {
			T* model = new T(this->sharedMesh);
			startPos += offset;
			Matrix4 currentTransform = model->GetTransform();
			currentTransform = Matrix4::Translation(startPos) * currentTransform;
			model->SetTransform(currentTransform);
			this->pool.push_back(model);
		}
	}

	void Update(float dt) override {
		for (const auto& model : this->pool) {
			model->Update(dt);

			Vector3 currentPos = model->GetWorldTransform().GetPositionVector();

			bool shouldRespawn = worldMoveDir.z < 0 ? currentPos.z < killPoint : currentPos.z > killPoint;
			if ( shouldRespawn ) {
				Matrix4 transform = model->GetTransform();
				Vector3 newPos = currentPos;
				newPos.z = this->spawnPoint.z;

				transform.SetPositionVector(newPos);
				model->SetTransform(transform);
			}
		}
	}

private:
	float killPoint;
	Vector3 worldMoveDir;
};

