#pragma once

#include "pch.h"

class Game;

class Win32Application {
public:
    static int Run(Game& game, HINSTANCE prevInstance, int cmdShow);

private:
    static HWND InitWindow(HINSTANCE hInstance, int cmdShow, Game& game);

    
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static void OnPaint(Game* pGame, HWND hWnd, WPARAM wParam, LPARAM lParam);
    static void OnSize(Game* pGame, HWND hWnd, WPARAM wParam, LPARAM lParam);

private:
	static bool s_in_sizemove;
	static bool s_in_suspend;
	static bool s_minimized;
	static bool s_fullscreen;
};