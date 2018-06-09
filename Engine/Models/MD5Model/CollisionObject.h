#pragma once

#include "pch.h"
using namespace DirectX;

namespace vxe {
	class CollisionObject {
	public:
		void InitBoundingBox()
		{
			auto floatMax = std::numeric_limits<float>::max();
			auto floatMin = std::numeric_limits<float>::lowest();

			vertexMin = DirectX::XMFLOAT3(floatMax, floatMax, floatMax);
			vertexMax = DirectX::XMFLOAT3(floatMin, floatMin, floatMin);
		}

		void UpdateBoundingBox(XMFLOAT3 vertex)
		{
			if (vertex.x < vertexMin.x) 
			{
				vertexMin.x = vertex.x;
			}
			if (vertex.y < vertexMin.y)
			{
				vertexMin.y = vertex.y;
			}
			if (vertex.z < vertexMin.z)
			{
				vertexMin.z = vertex.z;
			}
			if (vertex.x > vertexMax.x)
			{
				vertexMax.x = vertex.x;
			}
			if (vertex.y > vertexMax.y)
			{
				vertexMax.y = vertex.y;
			}
			if (vertex.z > vertexMax.z)
			{
				vertexMax.z = vertex.z;
			}
		}

		XMFLOAT3 GetMin() { return vertexMin; }
		XMFLOAT3 GetMax() { return vertexMax; }

	protected:
		XMFLOAT3 vertexMin;
		XMFLOAT3 vertexMax;
	};


}