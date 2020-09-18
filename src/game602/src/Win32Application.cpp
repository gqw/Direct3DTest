#include "Win32Application.h"

#include "game.h"

bool Win32Application::s_in_sizemove = false;
bool Win32Application::s_in_suspend = false;
bool Win32Application::s_minimized = false;
bool Win32Application::s_fullscreen = false;

int Win32Application::Run(Game& game, HINSTANCE prevInstance, int cmdShow)
{
    if (!DirectX::XMVerifyCPUSupport())
        return 1;

    if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED)))
        return 1;

    HWND hWnd = InitWindow(prevInstance, cmdShow, game);
    game.OnInit(hWnd);
	MSG msg = {};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
            game.OnRender();
		}
	}
	game.OnDestroy();
    return int(msg.wParam);
}

HWND Win32Application::InitWindow(HINSTANCE hInstance, int cmdShow, Game &game)
{
	WNDCLASSEX wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Win32Application::WndProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, _T("IDI_ICON"));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = game.className().data();
	wcex.hIconSm = LoadIcon(wcex.hInstance, _T("IDI_ICON"));
	if (!RegisterClassEx(&wcex))
		return nullptr;

	auto [w, h] = game.GetWindowSize();
	RECT rc = { 0, 0, LONG(w), LONG(h) };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	HWND hWnd = CreateWindowExW(0, game.className().data(), game.title().data(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		&game);
	if (hWnd == nullptr)
		return nullptr;

	ShowWindow(hWnd, cmdShow);
	UpdateWindow(hWnd);

    return hWnd;
}

LRESULT CALLBACK Win32Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto pGame = reinterpret_cast<Game *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pGame && pGame->OnMessage(hWnd, msg, wParam, lParam)) return true;

    switch (msg)
    {
    case  WM_CREATE: {
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        break;
    }
    case WM_PAINT:
    {
        Win32Application::OnPaint(pGame, hWnd, wParam, lParam);
        break;
    }
    case WM_SIZE:
        Win32Application::OnSize(pGame, hWnd, wParam, lParam);
        break;

    case WM_ENTERSIZEMOVE:
        s_in_sizemove = true;
        break;

    case WM_EXITSIZEMOVE:
        s_in_sizemove = false;
        if (pGame)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);

            pGame->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
        }
        break;

    case WM_KEYDOWN: {
        if (wParam == VK_SPACE) {
            if (pGame) pGame->OnChangeFullscreen();
        }
        break;
    }
    case WM_GETMINMAXINFO:
    {
        auto info = reinterpret_cast<MINMAXINFO *>(lParam);
        info->ptMinTrackSize.x = 320;
        info->ptMinTrackSize.y = 200;
    }
    break;

    case WM_ACTIVATEAPP:
        if (pGame)
        {
            if (wParam)
            {
                pGame->OnActivated();
            }
            else
            {
                pGame->OnDeactivated();
            }
        }
        break;

    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
            if (!s_in_suspend && pGame)
                pGame->OnSuspending();
            s_in_suspend = true;
            return TRUE;

        case PBT_APMRESUMESUSPEND:
            if (!s_minimized)
            {
                if (s_in_suspend && pGame)
                    pGame->OnResuming();
                s_in_suspend = false;
            }
            return TRUE;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Win32Application::OnPaint(Game* pGame, HWND hWnd, WPARAM wParam, LPARAM lParam) {
    static PAINTSTRUCT ps;
    if (s_in_sizemove && pGame)
    {
        pGame->OnRender();
    }
    else
    {
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
}

void Win32Application::OnSize(Game* pGame, HWND hWnd, WPARAM wParam, LPARAM lParam) {
	if (wParam == SIZE_MINIMIZED)
	{
		if (!s_minimized)
		{
			s_minimized = true;
			if (!s_in_suspend && pGame)
				pGame->OnSuspending();
			s_in_suspend = true;
		}
	}
	else if (s_minimized)
	{
		s_minimized = false;
		if (s_in_suspend && pGame)
			pGame->OnResuming();
		s_in_suspend = false;
	}
	else if (!s_in_sizemove && pGame)
	{
		pGame->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
	}
}