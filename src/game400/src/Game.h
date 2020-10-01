#pragma once
#include "pch.h"


class Game {
public:
    Game(tstring_view title, tstring_view className, UINT width, UINT height)
        : m_strTitle(title.data())
        , m_strClassName(className.data())
        , m_uiWidth(width)
        , m_uiHeight(height) {
    }

    bool OnInit(HWND hWnd);
    void OnRender();
    void OnDestroy();

    void OnActivated() {}
    void OnDeactivated() {}
    void OnSuspending() {}
    void OnResuming() {}
    void OnWindowSizeChanged(int width, int height);
    void OnChangeFullscreen();
    void OnDeviceLost();

    std::tuple<UINT, UINT> GetDefaultSize() { return {m_uiWidth, m_uiHeight}; }
    const tstring& title() { return m_strTitle; }
    const tstring& className() { return m_strClassName; }

private:
    void CreateDevice();
    void CreateResources();
    void CreateVectexShader();
    void CreatePixelShader();
    void CreateVertexBuffer();
    void Clear();
    void Present();

private:
    tstring m_strTitle;
    tstring m_strClassName;
    UINT m_uiWidth = 800;
    UINT m_uiHeight = 600;
    HWND m_hWnd = nullptr;
    BOOL m_isFullscreen = false;

    D3D_FEATURE_LEVEL m_freatureLevel = D3D_FEATURE_LEVEL_9_1;
	Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext;

	Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_vertexLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    
};