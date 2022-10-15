#pragma once

#include <d3d11.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
//#include <d3dx11tex.h>

class Direct3DTexture
{
public:
	Direct3DTexture();
	Direct3DTexture(const Direct3DTexture&);
	~Direct3DTexture();

	bool Initialize(ID3D11Device*, const WCHAR*);

	bool CreateBlankTexture(ID3D11Device* p_device);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

private:
	ID3D11ShaderResourceView* mTexture;
};