#include "pch.h"
#include "Win32Imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool Win32Imgui::OnInit(HWND hwnd, Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext) {
	m_d3dDevice = d3dDevice;
	m_d3dDeviceContext = d3dDeviceContext;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(m_d3dDevice.Get(), m_d3dDeviceContext.Get());
	return true;
}

void Win32Imgui::OnRender() {
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		// ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
	ImGui::Render();
	// m_d3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	// m_d3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Win32Imgui::OnDestory() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

bool Win32Imgui::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}