#include "pch.h"
#include "Win32Imgui.h"
#include "ViewManager.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool Win32Imgui::OnInit(HWND hwnd, Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext) {
	m_d3dDevice = d3dDevice;
	m_d3dDeviceContext = d3dDeviceContext;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(m_d3dDevice.Get(), m_d3dDeviceContext.Get());

	ImGui::GetIO().Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\simhei.ttf", 13.0f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());

	ViewManager::Inst().OnInit();

	return true;
}

void Win32Imgui::OnRender() {
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ViewManager::Inst().OnRender();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Win32Imgui::OnDestory() {
	ViewManager::Inst().OnDestory();
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

bool Win32Imgui::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}