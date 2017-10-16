#pragma once

#include "pch.h"
#include "../Mesh Base.h"
#include "../Third Party/DirectX Tool Kit/VertexTypes.h"

using namespace DirectX;

namespace vxe {

	struct Joint {
		std::wstring name;
		int parentId;

		XMFLOAT3 position;
		XMFLOAT4 orientation;
	};

	struct Weight {
		int jointId;
		float bias;
		XMFLOAT3 position;
	};

	class ModelSubset : public MeshBase<VertexPositionNormalTangentTextureWeight, unsigned short> {

	public:
		int texArrayIndex;
		int numberOfTriangles;

		std::vector<Weight> weights;
		std::vector<XMFLOAT3> positions;
	};

}