#include "pch.h"
#include "game.h"
#include "Win32Application.h"

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	LPWSTR cmdLine, int cmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(cmdLine);

    Game game(_T("TestGame1"), _T("TestGame1WndClass"), 800, 600);
    return Win32Application::Run(game, hInstance, cmdShow);
}