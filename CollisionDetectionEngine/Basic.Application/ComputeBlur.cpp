#include "pch.h"
#include "ComputeBlur.h"

using namespace DirectX;

ComputeBlur::ComputeBlur()
{
	m_writeUAV = 0;
	m_writeTexture = 0;
}

void ComputeBlur::Reset()
{
	if (m_writeUAV)
	{
		m_writeUAV->Release();
		m_writeUAV = 0;
	}
	if (m_writeTexture)
	{
		m_writeTexture->Release();
		m_writeTexture = 0;
	}
}

void ComputeBlur::Initialize(ID3D11Device2* device, int textureWidth, int textureHeight)
{
	auto textureDesc = createTexture2dDesc(textureWidth, textureHeight);
	device->CreateTexture2D(&textureDesc, NULL, &m_writeTexture);

	auto uavWDesc = createUAVDesc(textureDesc.Format);
	device->CreateUnorderedAccessView(m_writeTexture, &uavWDesc, &m_writeUAV);
	

	/*
	auto srcRDesc = createSRVDesc(textureDesc.Format);
	device->CreateShaderResourceView(m_readTexture, &srcRDesc, &m_readSRV);
	*/
}