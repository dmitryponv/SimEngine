#pragma once

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
//#pragma comment(lib, "d3dx10.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include "../../Containers.h"

using namespace DirectX;

class Direct3DContext
{
public:
	Direct3DContext();
	Direct3DContext(const Direct3DContext&);
	~Direct3DContext();

	bool Initialize(std::string viewport_name, std::shared_ptr<WindowContainer> window_container, bool, bool);
	bool Initialize_old(std::string viewport_name, std::shared_ptr<WindowContainer> window_container, bool, bool);
	void Shutdown();
	
	void BeginScene(std::string viewport_name, float, float, float, float);
	void EndScene(std::string viewport_name);

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void TurnZBufferOn();
	void TurnZBufferOff();

	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();

	ID3D11DepthStencilView* GetDepthStencilView();
	void SetBackBufferRenderTarget(std::string viewport_name);

	bool CreateNewSwapChain(std::string viewport_name, HWND *target_wnd);

private:
	bool m_vsync_enabled;
	
	std::map<std::string, IDXGISwapChain*> m_swapChain;
	std::map<std::string, ID3D11RenderTargetView*> m_renderTargetView;
	std::map<std::string, D3D11_VIEWPORT> m_viewPort;
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;

	ID3D11DepthStencilState* m_depthDisabledStencilState;
	ID3D11BlendState* m_alphaEnableBlendingState;
	ID3D11BlendState* m_alphaDisableBlendingState;
};