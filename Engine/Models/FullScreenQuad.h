#pragma once

#include "pch.h"

#include "..\Utilities.h"
#include "Mesh Base.h"

namespace vxe {
	template <typename T, typename U> class FullScreenQuad : public MeshBase<T, U> {};

	template<>
	class FullScreenQuad<DirectX::VertexPositionTexture, unsigned short> : public MeshBase<DirectX::VertexPositionTexture, unsigned short>{
	public:
		FullScreenQuad() {}
		//HardCoded
		virtual concurrency::task<void> CreateAsync(_In_ ID3D11Device2* device) override
		{
			std::vector<DirectX::VertexPositionTexture> vertices = {
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(-1.0f, -1.0f)),
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(-1.0f, 1.0f)),
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)),
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, -1.0f)),
			};
			std::vector<unsigned short> indices = {
				0, 1, 2,
				0, 2, 3
			};
			return MeshBase::CreateAsync(device, vertices, indices);
		}

		virtual concurrency::task<void> LoadAsync(_In_ ID3D11Device2* device, const std::wstring&) override
		{
			std::vector<DirectX::VertexPositionTexture> vertices = {
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(-1.0f, -1.0f)),
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(-1.0f, 1.0f)),
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)),
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, -1.0f)),
			};
			std::vector<unsigned short> indices = {
				0, 1, 2,
				0, 2, 3
			};
			return MeshBase::CreateAsync(device, vertices, indices);
		}

		virtual concurrency::task<void> CreateAsync(_In_ ID3D11Device2* device, const std::vector<char>&) override
		{
			std::vector<DirectX::VertexPositionTexture> vertices = {
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(-1.0f, -1.0f)),
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(-1.0f, 1.0f)),
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)),
				DirectX::VertexPositionTexture(DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, -1.0f)),
			};
			std::vector<unsigned short> indices = {
				0, 1, 2,
				0, 2, 3
			};
			return MeshBase::CreateAsync(device, vertices, indices);
		}


	};
}