#pragma once

#include "pch.h"
#include "../Mesh Base.h"
#include "../../Third Party/DirectX Tool Kit/VertexTypes.h"
#include "Md5AnimationStructs.h"

using namespace std;
using namespace concurrency;

namespace vxe{
	class Md5BoundingBoxMesh : public MeshBase<DirectX::VertexPositionColor, unsigned short> {
	
	public:
		Md5BoundingBoxMesh() 
		{
			std::vector<unsigned short> indices = {
				3, 1, 0,
				2, 1, 3,
				0, 5, 4,
				1, 5, 0,
				3, 4, 7,
				0, 4, 3,
				1, 6, 5,
				2, 6, 1,
				2, 7, 6,
				3, 7, 2,
				6, 4, 5,
				7, 4, 6,
			};

			_indices2 = indices;
		}

		concurrency::task<void> CreateAsync(ID3D11Device2 * device, BoundingBoxBorders* boundingBox)
		{
			_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			CreateMeshFromBoundingbox(boundingBox);
			return CreateAsync(device);
		}

		void UpdateBoundingBox(const BoundingBoxBorders* boundingBox)
		{
			_vertices2.clear();
			CreateMeshFromBoundingbox(boundingBox);
		}

		// Inherited via MeshBase
		virtual task<void> CreateAsync(ID3D11Device2 * device) override
		{
			return MeshBase::CreateAsync(device);
		}

		// Inherited via MeshBase
		virtual task<void> CreateAsync(ID3D11Device2 * device, const std::vector<char>& memory) override
		{
			return task<void>();
		}
	private:
		void CreateMeshFromBoundingbox(const BoundingBoxBorders* boundingBox)
		{
			auto min = boundingBox->min;
			auto max = boundingBox->max;

			std::vector<DirectX::VertexPositionColor> vertices = {
				DirectX::VertexPositionColor(XMFLOAT3(min.x, max.y, min.z), XMFLOAT4(Colors::Blue)),
				DirectX::VertexPositionColor(XMFLOAT3(max.x, max.y, min.z), XMFLOAT4(Colors::Blue)),
				DirectX::VertexPositionColor(XMFLOAT3(max.x, max.y, max.z), XMFLOAT4(Colors::Yellow)),
				DirectX::VertexPositionColor(XMFLOAT3(min.x, max.y, max.z), XMFLOAT4(Colors::Yellow)),
				DirectX::VertexPositionColor(XMFLOAT3(min.x, min.y, min.z), XMFLOAT4(Colors::Blue)),
				DirectX::VertexPositionColor(XMFLOAT3(max.x, min.y, min.z), XMFLOAT4(Colors::Blue)),
				DirectX::VertexPositionColor(XMFLOAT3(max.x, min.y, max.z), XMFLOAT4(Colors::Yellow)),
				DirectX::VertexPositionColor(XMFLOAT3(min.x, min.y, max.z), XMFLOAT4(Colors::Yellow))
			};
			_vertices2 = vertices;
		}
	};
}