#include "pch.h"
#include "game.h"
#include "Win32Application.h"

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	LPWSTR cmdLine, int cmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(cmdLine);

    std::shared_ptr<int> a = std::make_shared<int>(1);
    std::weak_ptr<int> b = a;
    auto c = b.lock();
    
    if (!logger::get().init("logs/" PROJECT_NAME ".log")) {
        return 1;
    }
    logger::get().set_level(spdlog::level::trace);

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