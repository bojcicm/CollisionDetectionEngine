#include "pch.h"
#include "RenderToScreen.h"
#include "ComputeBlur.h"

using namespace DirectX;

RenderScreenClass::RenderScreenClass()
{
	m_renderTargetTexture = 0;
	m_shaderResourceView = 0;
}

void RenderScreenClass::Initialize(ID3D11Device2* device, int textureWidth, int textureHeight, ID3D11Texture2D* texAddr)
{
	// SHADER RESOURCE VIEW
	auto textureDesc = createTexture2dDesc(textureWidth, textureHeight);
	auto shaderResourceViewDesc = createSRVDesv(textureDesc.Format);
	device->CreateShaderResourceView(texAddr, &shaderResourceViewDesc, &m_shaderResourceView);
}

void RenderScreenClass::Reset()
{
	if (m_renderTargetTexture)
	{
		m_renderTargetTexture->Release();
		m_renderTargetTexture = 0;
	}
	if (m_shaderResourceView)
	{
		m_shaderResourceView->Release();
		m_shaderResourceView = 0;
	}
}

ID3D11ShaderResourceView* RenderScreenClass::GetShaderResourceView()
{
	return m_shaderResourceView;
}

CD3D11_SHADER_RESOURCE_VIEW_DESC RenderScreenClass::getSRVDesc()
{
	return createSRVDesv(DXGI_FORMAT_R32G32B32A32_FLOAT);
}