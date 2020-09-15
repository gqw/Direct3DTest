#pragma once

#include <winsdkver.h>
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>


#include <windows.h>
#include <tchar.h>

#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <iostream>
#include <string>
#include <string_view>

#ifdef _UNICODE
#   define tstring std::wstring
#   define tstring_view std::wstring_view
#else
#   define tstring std::string
#   define tstring_view std::string_view
#endif