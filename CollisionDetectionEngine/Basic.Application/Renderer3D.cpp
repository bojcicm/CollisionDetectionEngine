#include "pch.h"

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
	
	_model = make_shared<MD5Model>();
	auto modelTasks = _model->CreateAsync(device, L"boblampclean.md5mesh", L"boblampclean.md5anim");
	tasks.insert(tasks.begin(), modelTasks.begin(), modelTasks.end());

	_cube = make_shared<CubeObject>();
	auto cubeTask = _cube->CreateAsync(device);
	tasks.push_back(cubeTask);

	_world = make_shared<WorldTransforms>(device);

	when_all(tasks.end(), tasks.end()).then([this]()
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
	static const XMVECTORF32 eye = { 0.0f, 70.0f, 110.0f, 0.0f };
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
	_projection = make_shared<ProjectionTransform>(device, Handedness::LeftHanded);
	_projection->SetProjection(orientationMatrix, fov, r, n, f);
	_projection->Update(context);
}

void Renderer3D::Update(DX::StepTimer const& timer)
{
	if (!m_loadingComplete) {
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();
	_model->Update(timer);
	_model->UpdateBuffers(context);

	//auto xMov = timer.GetElapsedSeconds();
	//_cube->TranslateObject(0.5f, 0.0f, 0.0f);
	
	/*auto isCollision = BoundingBoxCollisionTest(_model, _cube);
	if (isCollision)
	{
		DebugPrint(std::string("Collision happening\n"));
	}*/
}

void Renderer3D::Render()	
{
	if (!m_loadingComplete) {
		return;
	}
	auto context = m_deviceResources->GetD3DDeviceContext();

	SetCamera();

	_vertexshader->Bind(context);
	_pixelshader->Bind(context);

	_model->Render(context);
	//_cube->Render(context);

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
	_cube->Reset();

}

bool Renderer3D::BoundingBoxCollisionTest(shared_ptr<GameObject> object1, shared_ptr<GameObject> object2)
{
	auto vert1Min = XMVector3TransformCoord(XMLoadFloat3(&object1->GetMin()), XMLoadFloat4x4(&object1->GetWorldTransform()->GetWorld()));
	auto vert1Max = XMVector3TransformCoord(XMLoadFloat3(&object1->GetMax()), XMLoadFloat4x4(&object1->GetWorldTransform()->GetWorld()));
	auto vert2Min = XMVector3TransformCoord(XMLoadFloat3(&object2->GetMin()), XMLoadFloat4x4(&object2->GetWorldTransform()->GetWorld()));
	auto vert2Max = XMVector3TransformCoord(XMLoadFloat3(&object2->GetMax()), XMLoadFloat4x4(&object2->GetWorldTransform()->GetWorld()));

	DebugPrint(string("Testing collision between model and cube:\n"));
	DebugPrint(string("Model min:\t" + ToString(vert1Min) + "\n"));
	DebugPrint(string("Model max:\t" + ToString(vert1Max) + "\n"));
	DebugPrint(string("Cube min:\t" + ToString(vert2Min) + "\n"));
	DebugPrint(string("Cube max:\t" + ToString(vert2Max) + "\n"));

	if (XMVectorGetX(vert1Max) > XMVectorGetX(vert2Min))
		if (XMVectorGetX(vert1Min) < XMVectorGetX(vert2Max))
			if (XMVectorGetY(vert1Max) < XMVectorGetY(vert2Min))
				if (XMVectorGetY(vert1Min) < XMVectorGetY(vert2Max))
					if (XMVectorGetZ(vert1Max) < XMVectorGetZ(vert2Min))
						if (XMVectorGetZ(vert1Min) < XMVectorGetZ(vert2Max))
							return true;
	return false;
}