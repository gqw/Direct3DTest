#pragma once

#include "IMM/TsfInputMethodStore.h"

class Game;
class Win32Imgui
{
public:
	Win32Imgui(Game* game) : m_pGame(game) {};
	bool OnInit(HWND hwnd, Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext);

	void OnRender();

	void OnDestory();

	LRESULT OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
protected:
private:
	Game* m_pGame = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3dDeviceContext;

	/// Win32Imm m_imm;
};