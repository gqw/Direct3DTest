#pragma once
#include "pch.h"
#include <tuple>

class Game {
public:
    Game(tstring_view title, tstring_view className, UINT width, UINT height)
        : m_strTitle(title.data())
        , m_strClassName(className.data())
        , m_uiWidth(width)
        , m_uiHeight(height) {
    }

    bool OnInit(HWND hWnd);
    void OnRender();
    void OnDestroy();

    void OnActivated() {}
    void OnDeactivated() {}
    void OnSuspending() {}
    void OnResuming() {}
    void OnWindowSizeChanged(int width, int height) {}

    std::tuple<UINT, UINT> GetDefaultSize() { return {m_uiWidth, m_uiHeight}; }
    const tstring& title() { return m_strTitle; }
    const tstring& className() { return m_strClassName; }

private:
    tstring m_strTitle;
    tstring m_strClassName;
    UINT m_uiWidth = 800;
    UINT m_uiHeight = 600;
    HWND m_hWnd = nullptr;
};