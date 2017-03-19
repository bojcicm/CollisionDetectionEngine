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

	_screenVertexShader = make_shared<VertexShader<DirectX::VertexPositionColor>>();
	tasks.push_back(_screenVertexShader->CreateAsync(device, L"ScreenVertexShader.cso"));
	_screenPixelShader = make_shared<PixelShader>();
	tasks.push_back(_screenPixelShader->CreateAsync(device, L"ScreenPixelShader.cso"));

	//CS task
	_computeShader = make_shared<ComputeShader>();
	tasks.push_back(_computeShader->CreateAsync(device, L"ComputeShader.cso"));

	_verticalComputeShader = make_shared<ComputeShader>();
	tasks.push_back(_verticalComputeShader->CreateAsync(device, L"VerticalComputeShader.cso"));
	
	_model = make_shared<ExplicitSurface<VertexPositionColor,
	unsigned short >> (200.0f, 200.0f, 200, 200, [](float x, float z)
	{
		return (10 * expf(-(powf(x, 2.0f)) / 16.0f) * expf(-(powf(z, 2.0f)) / 16.0f))- 2*sin(x)*cos(z) ;
	});
	
	tasks.push_back(_model->CreateAsync(device));

	_fullScreenQuad = make_shared<FullScreenQuad<DirectX::VertexPositionTexture, unsigned short>>();
	tasks.push_back(_fullScreenQuad->CreateAsync(device));

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

	DebugPrint(string("\t\t Width:") + to_string(outputSize.Width) + string("\n"));
	DebugPrint(string("\t\t Height:") + to_string(outputSize.Height) + string("\n"));

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

	renderToTexture = make_shared<RenderTextureClass>();
	renderToTexture->Initialize(device, outputSize.Width, outputSize.Height);
	
	computeBlur = make_shared<ComputeBlur>();
	computeBlur->Initialize(device, outputSize.Width, outputSize.Height);

	renderToScreen = make_shared<RenderScreenClass>();
	renderToScreen->Initialize(device, outputSize.Width, outputSize.Height, computeBlur->GetTextureBindedToUAV());

}

void Renderer3D::RenderingToTexture()
{
	DebugPrint(std::string("RenderingToTexture() called\n"));
	auto context = m_deviceResources->GetD3DDeviceContext();

	
	renderToTexture->SetRenderTarget(context);
	renderToTexture->ClearRenderTarget(context, 0.0f, 0.0f, 0.0f, 1.0f);
	
	SetCamera();

	_vertexshader->Bind(context);
	_pixelshader->Bind(context);

	Draw(_model, _world);

	ID3D11RenderTargetView* pNullRTV = nullptr;
	context->OMSetRenderTargets(1, &pNullRTV, NULL);
}

void Renderer3D::Compute()
{
	DebugPrint(std::string("Renderer3D::Compute() ...\n"));

	auto context = m_deviceResources->GetD3DDeviceContext();

	auto renderedSRV = renderToTexture->GetShaderResourceView();
	context->CSSetShaderResources(0, 1, &renderedSRV);

	auto computeUAV = computeBlur->GetUnorderedAccessView();
	context->CSSetUnorderedAccessViews(0, 1, &computeUAV, 0);

	_computeShader->Bind(context);

	unsigned gx = computeBlur->GetWidth() / 30;
	unsigned gy = computeBlur->GetHeight();
	DebugPrint("Compute(" + std::to_string(gx) + ", " + std::to_string(gy) + ") ...\n");

	context->Dispatch(gx, gy, 1);

	//clear uav
	ID3D11UnorderedAccessView* pNullUAV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &pNullUAV, NULL);
}

void Renderer3D::RenderToScreen()
{
	DebugPrint(std::string("Renderer3D::RenderToScreen() ...\n"));

	auto context = m_deviceResources->GetD3DDeviceContext();
	auto device = m_deviceResources->GetD3DDevice();

	DebugPrint(std::string("Set render target to backbuffer and set UAV on PS\n"));
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	auto depthStencil = m_deviceResources->GetDepthStencilView();
	
	context->OMSetRenderTargets(1, targets, depthStencil);
	
	/*auto uavAddr = computeBlur->GetUnorderedAccessView();
	context->OMSetRenderTargetsAndUnorderedAccessViews(1, targets, depthStencil, 1, 1, &uavAddr, NULL);
	*/
		
	
	DebugPrint(std::string("Bind text2d to PS resource"));

	ID3D11ShaderResourceView* screenSRV= renderToScreen->GetShaderResourceView();
	context->PSSetShaderResources(0, 1, &screenSRV);
	

	DebugPrint(std::string("Bind screen VS and PS\n"));

	_screenVertexShader->Bind(context);
	_screenPixelShader->Bind(context);

	DebugPrint(std::string("Draw img from UAV\n"));

	Draw(_fullScreenQuad, _world);

	ID3D11RenderTargetView* pNullRTV = nullptr;
	context->OMSetRenderTargets(1, &pNullRTV, NULL);
}

void Renderer3D::Render()
{
	if (!m_loadingComplete) {
		return;
	}

	RenderingToTexture();

	Compute();

	RenderToScreen();

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
	_fullScreenQuad->Reset();

	_screenPixelShader->Reset();
	_screenVertexShader->Reset();

	renderToTexture->Reset();
	computeBlur->Reset();
	renderToScreen->Reset();
}