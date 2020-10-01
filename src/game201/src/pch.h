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
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl/client.h>

#include <codecvt>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <atlstr.h>
#include <strsafe.h>
#include <msctf.h>
#include <Commctrl.h>
#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#ifdef _UNICODE
#   define tstring std::wstring
#   define tstring_view std::wstring_view
#   define to_tstring std::to_wstring
#   define tstring_to_string(str) wstring_to_utf8(str)
#else
#   define tstring std::string
#   define tstring_view std::string_view
#   define to_tstring std::to_string
#   define tstring_to_string(str) (str)
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

#define THROW_IF_FAILED(hr) ThrowIfFailed(_T(__FILE__), __LINE__, hr)