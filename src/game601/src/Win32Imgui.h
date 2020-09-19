#pragma once

class Win32Imgui
{
public:
	bool OnInit(HWND hwnd, Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext);

	void OnRender();

	void OnDestory();

	bool OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
protected:
private:
	Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3dDeviceContext;
};