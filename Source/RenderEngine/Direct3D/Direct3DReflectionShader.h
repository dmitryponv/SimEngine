#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
//#include <d3dx11async.h>
#include <memory>
#include <fstream>
using namespace std;
using namespace DirectX;


class Direct3DReflectionShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct ReflectionBufferType
	{
		XMMATRIX reflectionMatrix;
	};

public:
	Direct3DReflectionShader();
	Direct3DReflectionShader(const Direct3DReflectionShader&);
	~Direct3DReflectionShader();

	bool Initialize(ID3D11Device*, std::shared_ptr<HWND> v_hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&, ID3D11ShaderResourceView*,
				ID3D11ShaderResourceView*, const XMMATRIX&);

private:
	bool InitializeShader(ID3D11Device*, std::shared_ptr<HWND> v_hwnd, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, std::shared_ptr<HWND> v_hwnd, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&, ID3D11ShaderResourceView*,
							 ID3D11ShaderResourceView*, const XMMATRIX&);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_reflectionBuffer;
};