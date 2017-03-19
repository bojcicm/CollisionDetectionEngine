#include "pch.h"

class ComputeBlur
{
public:
	ComputeBlur();

	void Initialize(ID3D11Device2*, int, int);
	void Reset();
	ID3D11UnorderedAccessView* GetUnorderedAccessView()
	{
		return m_writeUAV;
	}
	ID3D11Texture2D* GetTextureBindedToUAV()
	{
		return m_writeTexture;
	}
	void SetWidthHeight(int w, int h)
	{
		width = w;
		height = h;
	}
	int GetWidth()
	{
		return width;
	}
	int GetHeight()
	{
		return height;
	}

private:
	ID3D11UnorderedAccessView* m_writeUAV;
	
	int width;
	int height;
	
	ID3D11Texture2D* m_writeTexture; //bind to uav

	CD3D11_TEXTURE2D_DESC createTexture2dDesc(int width, int height)
	{
		SetWidthHeight(width, height);

		CD3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		return textureDesc;
	}
	CD3D11_UNORDERED_ACCESS_VIEW_DESC createUAVDesc(DXGI_FORMAT format){
		CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		return uavDesc;
	};
};