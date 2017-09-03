#include "pch.h"
#include "Renderer3D.h"

#include "..\..\..\Engine\Models\Basic Shapes\Pyramids.h"
#include "..\..\..\Engine\Models\Basic Shapes\Cubes.h"
#include "..\..\..\Engine\Models\Basic Shapes\Triangles.h"
#include "..\..\..\Engine\Models\Explicit Surface.h"
#include "..\..\Engine\FullScreenQuad.h"

using namespace std;
using namespace vxe;
using namespace concurrency;
using namespace DirectX;
using namespace Windows::Foundation;

void Renderer3D::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();
	vector<task<void>> tasks;
	
	_vertexshader = make_shared<VertexShader<DirectX::VertexPositionColor>>();
	tasks.push_back(_vertexshader->CreateAsync(device, L"VertexShader.cso"));
	_pixelshader = make_shared<PixelShader>();
	tasks.push_back(_pixelshader->CreateAsync(device, L"PixelShader.cso"));
	
	_model = make_shared<ExplicitSurface<VertexPositionColor,
	unsigned short >> (200.0f, 200.0f, 200, 200, [](float x, float z)
	{
		return (10 * expf(-(powf(x, 2.0f)) / 16.0f) * expf(-(powf(z, 2.0f)) / 16.0f))- 2*sin(x)*cos(z) ;
	});
	
	tasks.push_back(_model->CreateAsync(device));

	_world = make_shared<WorldTransforms>(device);

	when_all(tasks.begin(), tasks.end()).then([this]()
	{
		m_loadingComplete = true;
		DebugPrint(string("\t -- A lambda: Loading is complete! \n"));
	});
}

void Renderer3D::CreateWindowSizeDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();
	Size outputSize = m_deviceResources->GetOutputSize();

	_view = make_shared<ViewTransform>(device);
	static const XMVECTORF32 eye = { 0.0f, 10.0f, 15.0f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
	_view->SetView(eye, at, up);
	_view->Update(context);

	float r = outputSize.Width / outputSize.Height;
	float fov = 70.0f * XM_PI / 180.0f;
	float n = 0.01f;
	float f = 1000.0f;
	if (r < 1.0f) { fov *= 2.0f; }
	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);
	_projection = make_shared<ProjectionTransform>(device);
	_projection->SetProjection(orientationMatrix, fov, r, n, f);
	_projection->Update(context);

}

void Renderer3D::Render()
{
	if (!m_loadingComplete) {
		return;
	}

	DebugPrint(std::string("Renderer3D::Render() called\n"));
	auto context = m_deviceResources->GetD3DDeviceContext();

	SetCamera();

	_vertexshader->Bind(context);
	_pixelshader->Bind(context);

	Draw(_model, _world);

	m_deviceResources->SetRasterizerState();
}

void Renderer3D::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	DebugPrint(string("\tRenderer3D::ReleaseDeviceDependentResources()...\n"));

	_vertexshader->Reset();
	_pixelshader->Reset();
	_world->Reset();

	_model->Reset();
}