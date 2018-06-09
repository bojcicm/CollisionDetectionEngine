#pragma once
#include "pch.h"

#include "../../Scene/Transforms/World Transforms.h"
#include "../../Scene/Transforms/Position.h"
#include "CollisionObject.h"

namespace vxe {
	
	class GameObject : public CollisionObject {
	public:
		shared_ptr<WorldTransforms> GetWorldTransform() { return _localWorld; }
		shared_ptr<Position> GetPosition() { return _worldPosition; }

		void PositionDiff(float x, float y, float z)
		{
			_worldPosition->ChangePosition(x, y, z);
		}

		void ScaleDiff(float x, float y, float z)
		{
			_scale->ChangePosition(x, y, z);
		}

		void UpdateLocalWorld()
		{
			auto scaleMatrix = DirectX::XMMatrixScalingFromVector(_scale->GetPosition());
			auto positionMatrix = DirectX::XMMatrixTranslationFromVector(_worldPosition->GetPosition());
			auto rotationMatrix = DirectX::XMMatrixRotationX(0.0f);
			_localWorld->Transform(scaleMatrix, rotationMatrix, positionMatrix);
		}

	protected:
		shared_ptr<WorldTransforms> _localWorld;
		shared_ptr<Position> _worldPosition;
		shared_ptr<Position> _scale;
	};
}