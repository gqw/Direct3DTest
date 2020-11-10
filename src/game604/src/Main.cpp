#include "pch.h"
#include "game.h"
#include "Win32Application.h"

#include "imgui_internal.h"

int WStringLenToUtf8Length1(char const* str, int wend) {
	int len = 0, index = 0;
	while (index < wend) {
		if (*str == '\0') break;

		index++;
		if ((*str & 0x80) == 0) {
			str++;
			len += 1;
		}
		else if ((*str & 0xF0) == 0xF0) {
			str += 4;
			len += 4;
		}
		else if ((*str & 0xE0) == 0xE0) {
			str += 3;
			len += 3;
		}
		else if ((*str & 0xC0) == 0xC0) {
			str += 2;
			len += 2;
		}
	}
	return len;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	LPWSTR cmdLine, int cmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(cmdLine);

	std::string s = wstring_to_utf8(L"好");
	int len = WStringLenToUtf8Length1(s.c_str(), 15);

	ImTextCountCharsFromUtf8(s.c_str(), s.c_str() + 3);

    std::shared_ptr<int> a = std::make_shared<int>(1);
    std::weak_ptr<int> b = a;
    auto c = b.lock();
    
    if (!logger::get().init("logs/test.log")) {
        return 1;
    }
    logger::get().set_level(spdlog::level::debug);

	// LOG_INFO("test {}", 1);
	// PRINT_INFO("test print log, %d", 55);
	// STM_DEBUG() << "debug test";
	// logger::get().set_level(spdlog::level::trace);
	// STM_DEBUG() << "debug test2";
	if FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)) {
		return 1;
	}
	auto& game = Game::Inst();
	game.Init(_T(PROJECT_NAME), _T(PROJECT_NAME"WndClass"), 800, 600);
	return Win32Application::Run(game, hInstance, cmdShow);
    try
    {
        if FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)) {
            return 1;
        }
        auto& game = Game::Inst();
        game.Init(_T(PROJECT_NAME), _T(PROJECT_NAME"WndClass"), 800, 600);
		return Win32Application::Run(game, hInstance, cmdShow);
    }
    catch (std::exception e)
    {
        STM_WARN() << e.what();
    }
    CoUninitialize();
    return 0;
}