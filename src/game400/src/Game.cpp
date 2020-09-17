#include "game.h"
#include <algorithm>
#ifdef WIN32
#undef min
#undef max
#endif

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct SimpleVertex
{
	XMFLOAT3 Pos;
};

bool Game::OnInit(HWND hWnd) {
    m_hWnd = hWnd;

    CreateDevice();
    CreateResources();
    return true;
}

void Game::OnRender() {
    Clear();

	CD3D11_VIEWPORT viewport(0.0f, 0.0f, float(m_uiWidth), float(m_uiHeight));
	m_d3dContext->RSSetViewports(1, &viewport);

	// 不能使用&m_renderTargetView operator &会调用 InternalRelease()， 导致m_renderTargetView为空
	m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_d3dContext->IASetInputLayout(m_vertexLayout.Get());
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
    m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    m_d3dContext->IASetInputLayout(m_vertexLayout.Get());
	m_d3dContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_d3dContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    m_d3dContext->Draw(3, 0);

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
        featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, &device,
        &m_freatureLevel, &context));

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

    CreateVectexShader();
    CreatePixelShader();  
}

void Game::CreateResources() {
    if (m_d3dDevice == nullptr) return;
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
    CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, m_uiWidth, m_uiHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);
    THROW_IF_FAILED(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil));
    
    CD3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    THROW_IF_FAILED(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthViewDesc, &m_depthStencilView));

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

	CD3D11_VIEWPORT viewport(0.0f, 0.0f, float(m_uiWidth), float(m_uiHeight));
	m_d3dContext->RSSetViewports(1, &viewport);
}

void Game::Clear() {
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
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

void Game::OnChangeFullscreen() {
    if (m_swapChain == nullptr) return;

    BOOL isFullscreen = FALSE;
    THROW_IF_FAILED(m_swapChain->GetFullscreenState(&isFullscreen, nullptr));
    THROW_IF_FAILED(m_swapChain->SetFullscreenState(!isFullscreen, nullptr));
}

void Game::OnWindowSizeChanged(int width, int height) {
	m_uiWidth = std::max(width, 1);
	m_uiHeight = std::max(height, 1);

    CreateResources();
}

void Game::CreateVectexShader() {
	// create vs shader
    ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> errorBlob;
	std::string vertexShader = R"(
        float4 VS( float4 Pos : POSITION ) : SV_POSITION
        {
            return Pos;
        }
    )";

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	THROW_IF_FAILED(D3DCompile(vertexShader.c_str(), vertexShader.length(), nullptr,
		nullptr, nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &shaderBlob, &errorBlob));

    m_d3dDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &m_vertexShader);
    
	// create layout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	UINT numElements = _countof(layout);
    THROW_IF_FAILED(m_d3dDevice->CreateInputLayout(layout, _countof(layout), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &m_vertexLayout));
    m_d3dContext->IASetInputLayout(m_vertexLayout.Get());
    CreateVertexBuffer();
}

void Game::CreatePixelShader() {
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob = nullptr;
	std::string pixelShader = R"(
        float4 PS( float4 Pos : SV_POSITION ) : SV_Target
        {
            return float4( 1.0f, 1.0f, 0.0f, 1.0f );    // Yellow, with Alpha = 1
        }
    )";
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	THROW_IF_FAILED(D3DCompile(pixelShader.c_str(), pixelShader.length(), nullptr,
		nullptr, nullptr, "PS", "ps_4_0", dwShaderFlags, 0, &shaderBlob, &errorBlob));

    THROW_IF_FAILED(m_d3dDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &m_pixelShader));
}


void Game::CreateVertexBuffer() {
    SimpleVertex vertices[] = {
        XMFLOAT3{0.0f, 0.5f, 0.5f},
        XMFLOAT3{0.5f, -0.5f, 0.5},
        XMFLOAT3{-0.5f, -0.5f, 0.5f},
    };
    CD3D11_BUFFER_DESC bd(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT);
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;
    THROW_IF_FAILED(m_d3dDevice->CreateBuffer(&bd, &initData, &m_vertexBuffer));

    // Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set primitive topology
    m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}