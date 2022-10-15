#include "Direct3DTexture.h"
#include "DDSTextureLoader.h"


Direct3DTexture::Direct3DTexture()
{
	mTexture = 0;
	uint32_t s_pixel = (uint32_t)rand();
}


Direct3DTexture::Direct3DTexture(const Direct3DTexture& other)
{
}


Direct3DTexture::~Direct3DTexture()
{
}


bool Direct3DTexture::Initialize(ID3D11Device* device, const WCHAR* file_name)
{
	HRESULT result;
	
	result = DirectX::CreateDDSTextureFromFile(device, file_name, NULL, &mTexture, NULL);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


bool Direct3DTexture::CreateBlankTexture(ID3D11Device * p_device)
{
	//srand(time(0));
	uint32_t s_pixel = (uint32_t)rand() | 0xa0a0a0a0;// | 0xff000000;// 0xffafbfcf;

	D3D11_SUBRESOURCE_DATA initData = { &s_pixel, sizeof(uint32_t), 0 };

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = desc.Height = desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D* tex;
	HRESULT hr = p_device->CreateTexture2D(&desc, &initData, &tex);

	if (SUCCEEDED(hr))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = p_device->CreateShaderResourceView(tex,&SRVDesc, &mTexture);
	}

	if (FAILED(hr))
		return false;
	return true;
}

void Direct3DTexture::Shutdown()
{
	// Release the texture resource.
	if(mTexture)
	{
		mTexture->Release();
		mTexture = 0;
	}

	return;
}


ID3D11ShaderResourceView* Direct3DTexture::GetTexture()
{
	return mTexture;
}