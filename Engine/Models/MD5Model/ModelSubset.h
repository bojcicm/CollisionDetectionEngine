#pragma once

#include "pch.h"
#include "../Mesh Base.h"
#include "../../Third Party/DirectX Tool Kit/VertexTypes.h"

namespace vxe {

	struct Joint {
		std::wstring name;
		int parentId;

		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 orientation;
	};

	struct Weight {
		int jointId;
		float bias;
		DirectX::XMFLOAT3 position;
	};

	class ModelSubset : public MeshBase<DirectX::VertexPositionNormalTangentTextureWeight, unsigned short> {

	public:

		ModelSubset() { }

		int texArrayIndex;
		int numberOfTriangles;

		std::vector<Weight> weights;
		std::vector<DirectX::XMFLOAT3> positions;

		//Hardcoded values
		virtual concurrency::task<void> CreateAsync(ID3D11Device2 * device)
		{
			DebugPrint(std::string("\t Cube<VertexPositionColor1, unsigned short>::CreateAsync() ...\n"));

			std::vector<DirectX::VertexPositionNormalTangentTextureWeight> vertices;
			std::vector<unsigned short> indices;

			return MeshBase::CreateAsync(device, vertices, indices);
		}

		// Creating from memory
		virtual concurrency::task<void> CreateAsync(_In_ ID3D11Device2* device, const std::vector<char>&)
		{
			DebugPrint(std::string("\t Cube<VertexPositionColor1, unsigned short>::CreateAsync() ...\n"));

			std::vector<DirectX::VertexPositionNormalTangentTextureWeight> vertices;
			std::vector<unsigned short> indices;

			return MeshBase::CreateAsync(device, vertices, indices);
		}

		concurrency::task<void> CreateAsync(_In_ ID3D11Device2* device,
			std::vector<DirectX::VertexPositionNormalTangentTextureWeight>& vertices,
			std::vector<unsigned short>& indices,
			D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
		{
			return MeshBase::CreateAsync(device, vertices, indices, topology);
		}

	};

}