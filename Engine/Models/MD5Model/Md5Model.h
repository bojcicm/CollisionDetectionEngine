#pragma once

#include "pch.h"

#include "../../Core/Common/StepTimer.h"
#include "../../Scene/Transforms/World Transforms.h"
#include "Md5Mesh.h"
#include "Md5Structs.h"
#include "Md5Animation.h"
#include "GameObject.h"

using namespace std;
using namespace concurrency;
using namespace DirectX;

namespace vxe {

	class MD5Model : public GameObject {

	public:
		MD5Model() {};
		
		vector<task<void>> CreateAsync(_In_ ID3D11Device2* device, wstring filename, wstring animationFileName = L"");

		void Update(DX::StepTimer const&);
		void UpdateBuffers(_In_ ID3D11DeviceContext2* context);
		void Render(_In_ ID3D11DeviceContext2* context);
		void RenderMesh(_In_ ID3D11DeviceContext2* context, const shared_ptr<Md5Mesh>& mesh);
		
		void Reset()
		{
			for (auto m : _meshes)
			{
				m->Reset();
			}
			_animation->Reset();
		}
			

	protected:
		void PrepareMesh(shared_ptr<Md5Mesh> mesh);
		void PrepareMesh(shared_ptr<Md5Mesh> mesh, shared_ptr<Md5Animation> animation);
		void PrepareNormals(shared_ptr<Md5Mesh> mesh);

		task<void> LoadMd5Model(ID3D11Device2 * device, wstring filename);
		task<void> LoadMd5Animation(wstring filename);

		int m_numberOfMeshes;
		int m_numberOfJoints;
		bool m_hasAnimation;
		JointList _joints;
		shared_ptr<Md5Animation> _animation;
		vector<shared_ptr<Md5Mesh>> _meshes;
	};
}