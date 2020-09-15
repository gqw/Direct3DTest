#include "game.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

bool Game::OnInit(HWND hWnd) {
    m_hWnd = hWnd;

    CreateDevice();
    CreateResources();
    return true;
}

void Game::OnRender() {
    Clear();

    Present();
}

void Game::OnDestroy() {

}

void Game::CreateDevice() {
    UINT creationFlags = 0;
#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;

    THROW_IF_FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
        featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, device.ReleaseAndGetAddressOf(),
        &m_freatureLevel, context.ReleaseAndGetAddressOf()));

#ifdef _DEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug))) {
		ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
		{
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
			};
			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
    }
#endif // _DEBUG
    THROW_IF_FAILED(device.As(&m_d3dDevice));
    THROW_IF_FAILED(context.As(&m_d3dContext));
}

void Game::CreateResources() {
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();


	UINT backBufferCount = 2;
    if (m_swapChain) {
        DXGI_SWAP_CHAIN_DESC1 desc;
        m_swapChain->GetDesc1(&desc);
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, m_uiWidth, m_uiHeight, desc.Format, desc.Flags);
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            return OnDeviceLost();
        }
        else 
        {
            THROW_IF_FAILED(hr);
        }
    }
    else 
    {
        ComPtr<IDXGIDevice1> dxgiDevice;
        THROW_IF_FAILED(m_d3dDevice.As(&dxgiDevice));

        ComPtr<IDXGIAdapter> dxgiAdapter;
        THROW_IF_FAILED(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        ComPtr<IDXGIFactory2> dxgiFactory;
        THROW_IF_FAILED(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = m_uiWidth;
		swapChainDesc.Height = m_uiHeight;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = backBufferCount;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
        fullscreenDesc.Windowed = TRUE;

        THROW_IF_FAILED(dxgiFactory->CreateSwapChainForHwnd(m_d3dDevice.Get(),
            m_hWnd, &swapChainDesc, &fullscreenDesc, nullptr, m_swapChain.ReleaseAndGetAddressOf()));

        dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
    }
    ComPtr<ID3D11Texture2D> backBuffer;
    THROW_IF_FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
    THROW_IF_FAILED(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView));

    ComPtr<ID3D11Texture2D> depthStencil;
    /*CD3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.Width = m_uiWidth;
    depthStencilDesc.Height = m_uiHeight;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    */
    CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, m_uiWidth, m_uiHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);
    THROW_IF_FAILED(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));
    
    CD3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    THROW_IF_FAILED(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthViewDesc, &m_depthStencilView));
}

void Game::Clear() {
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 不能使用&m_renderTargetView operator &会调用 InternalRelease()， 导致m_renderTargetView为空
    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    CD3D11_VIEWPORT viewport(0.0f, 0.0f, float(m_uiWidth), float(m_uiHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

void Game::Present() {
    HRESULT hr = m_swapChain->Present(1, 0);
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		OnDeviceLost();
	}
	else
	{
		THROW_IF_FAILED(hr);
	}
}


void Game::OnDeviceLost()
{
	m_depthStencilView.Reset();
	m_renderTargetView.Reset();
	m_swapChain.Reset();
	m_d3dContext.Reset();
	m_d3dDevice.Reset();

	CreateDevice();

	CreateResources();
}