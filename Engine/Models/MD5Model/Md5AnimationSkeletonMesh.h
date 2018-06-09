#pragma once

#include "pch.h"

#include "../Mesh Base.h"
#include "../../Third Party/DirectX Tool Kit/VertexTypes.h"
//#include "Md5Structs.h"

using namespace std;
using namespace concurrency;
using namespace DirectX;

namespace vxe {
	class Md5AnimationSkeletonMesh : public MeshBase<VertexPositionColor, unsigned short> {
	
	public:
		Md5AnimationSkeletonMesh() { }

		concurrency::task<void> CreateAsync(ID3D11Device2 * device, JointList* jointList)
		{
			_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			CreateSkeletonFromJointList(jointList);
			return CreateAsync(device);
		}

		void UpdateSkeletonMesh(const JointList* jointList)
		{
			_vertices2.clear();
			_indices2.clear();

			CreateSkeletonFromJointList(jointList);
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
		void CreateSkeletonFromJointList(const JointList* jointsList)
		{
			for (auto i = 0; i < jointsList->size(); i++)
			{
				auto& joint = (*jointsList)[i];
				auto vertex = VertexPositionColor(DirectX::XMLoadFloat3(&joint.position), DirectX::Colors::Yellow);
				_vertices2.push_back(vertex);

				if (joint.parentId != -1)
				{
					_indices2.push_back(i);
					_indices2.push_back(joint.parentId);
				}
			}
		}
	};
}