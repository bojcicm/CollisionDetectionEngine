#pragma once

#include "pch.h"

#include "../Mesh Base.h"
#include "../../Core/Common/StepTimer.h"
#include "../../Scene/Transforms/World Transforms.h"
#include "../../Third Party/DirectX Tool Kit/VertexTypes.h"
#include "Md5Structs.h"

using namespace std;
using namespace concurrency;
using namespace DirectX;

namespace vxe {
	class Md5Mesh : public MeshBase<VertexPositionNormalColorTexture, unsigned short> {

	public:

		Md5Mesh() { }

		int texArrayIndex;
		int numberOfTriangles;

		WeightList weights;
		VertexWeightInfoList vertexWeightsInfo;
		vector<DirectX::XMFLOAT3> positions;
		
		void PushBackVertex(VertexPositionNormalColorTexture vertex)
		{
			_vertices2.push_back(vertex);
		}

		void PushBackIndices(unsigned int index) 
		{
			_indices2.push_back(index);
		}

		virtual task<void> Md5Mesh::CreateAsync(ID3D11Device2 * device) override
		{
			return MeshBase::CreateAsync(device);
		}

		// Inherited via MeshBase
		virtual concurrency::task<void> CreateAsync(ID3D11Device2 * device, const std::vector<char>& memory) override
		{
			return concurrency::task<void>();
		}
	};
}