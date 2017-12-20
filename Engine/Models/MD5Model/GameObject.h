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

		void TranslateObject(float x, float y, float z)
		{
			_worldPosition->ChangePosition(x, y, z);
			auto newPosition = _worldPosition->GetValue();
			_localWorld->Translate(newPosition.x, newPosition.y, newPosition.z);
		}

	protected:
		shared_ptr<WorldTransforms> _localWorld;
		shared_ptr<Position> _worldPosition;
		shared_ptr<Position> _scale;
	};
}