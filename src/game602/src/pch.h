#pragma once

#include <winsdkver.h>
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>


#include <windows.h>
#include <tchar.h>

#include <comdef.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <DirectXTex.h>

#include <wrl/client.h>

#include <codecvt>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <list>
#include <map>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#ifdef _UNICODE
#   define tstring std::wstring
#   define tstring_view std::wstring_view
#   define to_tstring std::to_wstring
#   define tstring_to_string(str) wstring_to_utf8(str)

#	define D3DX11CompileFromFile D3DX11CompileFromFileW
#else
#   define tstring std::string
#   define tstring_view std::string_view
#   define to_tstring std::to_string
#   define tstring_to_string(str) (str)
#	define D3DX11CompileFromFile D3DX11CompileFromFileA
#endif

inline std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

inline std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

inline void ThrowIfFailed(tstring_view file, uint32_t lineNo, HRESULT hr) {
	if (SUCCEEDED(hr)) return;
	_com_error err(hr);
	tstring errInfo(file);
	errInfo = errInfo + _T("(") + to_tstring(lineNo) + _T("): HRESULT: ")
		+ to_tstring(hr) + _T(" ") + err.ErrorMessage();

	std::string pinfo = tstring_to_string(errInfo).c_str();
	throw std::logic_error(pinfo);
}

inline void ThrowLogicError(std::string_view file, uint32_t lineNo, std::string_view condition, std::string_view desc) {
	std::string errInfo(file);
	errInfo = errInfo + "(" + std::to_string(lineNo) + "): " + (condition.empty() ? "" : condition.data() + std::string(", ")) + desc.data();
	throw std::logic_error(errInfo);
}

#define THROW_IF_FAILED(hr) ThrowIfFailed(_T(__FILE__), __LINE__, hr)
#define THROW_ERROR(desc) ThrowLogicError(_T(__FILE__), __LINE__, "", (desc))
#define ASSERT_THROW(condition, desc) if (!(condition)) ThrowLogicError(__FILE__, __LINE__, (#condition), (desc))