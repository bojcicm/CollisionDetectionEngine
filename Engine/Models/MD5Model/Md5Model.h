#pragma once

#include "pch.h"

#include "../../Core/Common/StepTimer.h"
#include "../../Scene/Transforms/World Transforms.h"
#include "Md5Mesh.h"
#include "Md5Animation.h"
#include "Md5AnimationStructs.h"
#include "Md5Structs.h"

using namespace std;
using namespace concurrency;
using namespace DirectX;

namespace vxe {

	class MD5Model {

	public:
		MD5Model() {};
		virtual ~MD5Model() {};

		void Update(DX::StepTimer const&);
		void Reset();
		void Render(_In_ ID3D11DeviceContext2* context);
		task<void> CreateAsync(_In_ ID3D11Device2* device, wstring filename, wstring animationFileName = L"");

	protected:
		void PrepareMesh(shared_ptr<Md5Mesh> mesh);
		void PrepareNormals(shared_ptr<Md5Mesh> mesh);

		/*void RenderMesh();
		void RenderNormals();
		void RenderSkeleton();
		void CheckAnimation() const;*/
		task<void> LoadMd5Model(ID3D11Device2 * device, wstring filename);
		task<void> LoadMd5Animation(wstring filename);

		int m_numberOfMeshes;
		int m_numberOfJoints;
		bool m_hasAnimation;
		JointList _joints;
		vector<shared_ptr<Md5Mesh>> _meshes;
		shared_ptr<Md5Animation> _animation;

		shared_ptr<WorldTransforms> _localWorld;
	};
}