#include "pch.h"

class RenderScreenClass
{
public:
	RenderScreenClass();

	void Initialize(ID3D11Device2*, int, int, ID3D11Texture2D*);
	void Reset();

	void SetRenderTarget(ID3D11DeviceContext2*, std::shared_ptr<DX::DeviceResources>);
	ID3D11ShaderResourceView* GetShaderResourceView();
	CD3D11_SHADER_RESOURCE_VIEW_DESC getSRVDesc();

private:
	ID3D11Texture2D* m_renderTargetTexture;
	ID3D11ShaderResourceView* m_shaderResourceView;

	CD3D11_TEXTURE2D_DESC createTexture2dDesc(int width, int height)
	{
		CD3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		return textureDesc;
	}
	CD3D11_SHADER_RESOURCE_VIEW_DESC createSRVDesv(DXGI_FORMAT format)
	{
		CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		shaderResourceViewDesc.Format = format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		return shaderResourceViewDesc;
	}
};