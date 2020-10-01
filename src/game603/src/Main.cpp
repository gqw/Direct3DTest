#include "pch.h"
#include "game.h"
#include "Win32Application.h"


#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	LPWSTR cmdLine, int cmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(cmdLine);
	//ULONG_PTR gdiplustoken;
	//Gdiplus::GdiplusStartupInput gdiplusstartupinput;
	//Gdiplus::GdiplusStartup(&gdiplustoken, &gdiplusstartupinput, NULL);

    try
    {
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        auto& game = Game::Inst();
        game.Init(_T(PROJECT_NAME), _T(PROJECT_NAME"WndClass"), 800, 600);
		return Win32Application::Run(game, hInstance, cmdShow);
    }
    catch (std::exception e)
    {
        OutputDebugString(utf8_to_wstring(e.what()).c_str());
    }
    return 1;
}