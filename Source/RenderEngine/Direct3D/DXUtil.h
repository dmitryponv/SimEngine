#pragma once

#include <d3d11.h>
#include <DirectXColors.h>
#include "dxerr.h"
#include <vector>
#include <DirectXMath.h>
#include <string>

#pragma comment(lib, "d3d11.lib")

#ifdef _DEBUG
#ifndef HR
#define HR(xx) {HRESULT hr = xx; if (FAILED(hr)) {DXTraceW(__FILEW__, __LINE__, hr, L#xx, TRUE);}} 
#endif
#else
#ifndef HR
#define HR(x) x;
#endif
#endif

#ifndef PI
#define PI DirectX::XM_PI
#endif


namespace Memory
{
	template <class T> void SafeDelete(T& v_t)
	{
		if (v_t)
		{
			delete v_t;
			v_t = nullptr;
		}
	}

	template <class T> void SafeDeleteArr(T& v_t)
	{
		if (v_t)
		{
			delete[] v_t;
			v_t = nullptr;
		}
	}

	template <class T> void SafeRelease(T& v_t)
	{
		if (v_t)
		{
			v_t->Release();
			v_t = nullptr;
		}
	}
}

namespace Utility
{
	void GetTextureDim(ID3D11Resource* v_res, UINT* v_width, UINT* v_height);
	void PrintMatrix(std::string name, const DirectX::XMMATRIX& input_m);
}