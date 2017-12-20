#pragma once

#include "pch.h"
#include "GameObject.h"
#include "../Basic Shapes/Cubes.h"
#include "../../Third Party/DirectX Tool Kit/VertexTypes.h"

using namespace std;
using namespace concurrency;

namespace vxe {
	class CubeObject : public GameObject {
	public:

		task<void> CreateAsync(ID3D11Device2 * device)
		{
			_worldPosition = std::make_shared<Position>(device);
			_localWorld = std::make_shared<WorldTransforms>(device);
			_localWorld->Scale(10.0f, 10.0f, 10.0f);

			cube = make_shared<Cube<VertexPositionColor, unsigned short>>();
			return cube->CreateAsync(device);
		}

		void Render(_In_ ID3D11DeviceContext2* context)
		{
			_localWorld->Update(context);
			_localWorld->GetConstantBuffer()->Bind(context);

			cube->BindVertexBuffer(context);
			cube->BindIndexBuffer(context);
			cube->DrawIndexed(context);
		}

		void Reset()
		{
			cube->Reset();
		}

	protected:
		shared_ptr<Cube<VertexPositionColor, unsigned short>> cube;
	};
}