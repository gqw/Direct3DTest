#pragma once
#include "pch.h"
#include "Win32Imgui.h"

#define  WM_USER_FULLSCREEN  (WM_USER + 1000)
class Game {
public:
    static Game& Inst() { static Game game; return game; }

    bool Init(tstring_view title, tstring_view className, UINT width, UINT height);
    bool OnInit(HWND hWnd);
    void OnRender();
    void OnDestroy();

    bool OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnActivated() {}
    void OnDeactivated() {}
    void OnSuspending() {}
    void OnResuming() {}
    void OnWindowSizeChanged(int width, int height);
    void OnChangeFullscreen();
    void OnDeviceLost();

    std::tuple<UINT, UINT> GetWindowSize() { return {m_uiWidth, m_uiHeight}; }
    UINT width() { return m_uiWidth; }
    UINT height() { return m_uiHeight; }
    const tstring& title() { return m_strTitle; }
    const tstring& className() { return m_strClassName; }
    BOOL fullscreen() { return m_isFullscreen; }
    HWND window_handle() { return m_hWnd; }

private:
    void CreateDevice();
    void CreateResources(bool forcecreate = false);
    void CreateVectexShader();
    void CreatePixelShader();
    void CreateVertexBuffer();
    void LoadBackgroundPng();

    void Clear();
    void Present();

private:
    tstring m_strTitle;
    tstring m_strClassName;
    UINT m_uiWidth = 800;
    UINT m_uiHeight = 600;
    HWND m_hWnd = nullptr;
    BOOL m_isFullscreen = FALSE;

    D3D_FEATURE_LEVEL m_freatureLevel = D3D_FEATURE_LEVEL_9_1;
	Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext;

	Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  m_backgroundView;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerLinear;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_vertexLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    
    std::unique_ptr<Win32Imgui>                     m_win32Imgui;   
};