#pragma once

#include "pch.h"

#include "..\..\..\Engine\Third Party\DirectX Tool Kit\VertexTypes.h"

#include "..\..\..\Engine\Pipeline Stages\Shaders\Vertex Shader.h"
#include "..\..\..\Engine\Pipeline Stages\Shaders\Pixel Shader.h"
#include "..\..\Engine\Compute Shader.h"

#include "..\..\..\Engine\Models\Mesh Base.h"
#include "..\..\Engine\Models\Basic Shapes\Cubes.h"
#include "..\..\..\Engine\Models\MD5Model\Md5Model.h"

#include "..\..\..\Engine\Scene\Transforms\World Transforms.h"
#include "..\..\..\Engine\Scene\Transforms\View Transform.h"
#include "..\..\..\Engine\Scene\Transforms\Projection Transform.h"

using namespace vxe;

class Renderer3D :public RendererBase3D
{
public:
	std::shared_ptr<VertexShader<DirectX::VertexPositionColor>> _vertexshader;
	std::shared_ptr<PixelShader> _pixelshader;

	std::shared_ptr<MD5Model> _model;

	std::shared_ptr<WorldTransforms> _world;
	std::shared_ptr<ViewTransform> _view;
	std::shared_ptr<ProjectionTransform> _projection;

	Renderer3D() :RendererBase3D()
	{
		DebugPrint(std::string("Renderer3D::Ctor called\n"));
	}
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void Update(DX::StepTimer const & timer);
	void ReleaseDeviceDependentResources();
	void Render();
	void Rotate(float radians)
	{
		_world->RotateY(radians);
	}
	template <typename T, typename U>
	void Draw(std::shared_ptr<MeshBase<T, U>>& mesh,
		std::shared_ptr<WorldTransforms>& world,
		bool indexed = true)
	{
		auto context = m_deviceResources->GetD3DDeviceContext();
		world->Update(context);
		world->GetConstantBuffer()->Bind(context);
		mesh->BindVertexBuffer(context);
		mesh->BindIndexBuffer(context);
		if (indexed) mesh->DrawIndexed(context);
		else mesh->Draw(context);
	}

private:
	void SetCamera()
	{
		DebugPrint(std::string("Renderer3D::SetCamera() called\n"));
		auto context = m_deviceResources->GetD3DDeviceContext();
		_view->GetConstantBuffer()->Bind(context, ProgrammableStage::VertexShaderStage, 1);
		_projection->GetConstantBuffer()->Bind(context, ProgrammableStage::VertexShaderStage, 2);
	}
};
