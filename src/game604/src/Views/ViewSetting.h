#pragma once

#include "../pch.h"
#include "../ViewInterface.h"

class ViewSetting : public ViewInterface
{
public:
    ViewSetting(std::string_view name) : ViewInterface(name) {};
    virtual ~ViewSetting() {}

    virtual bool OnInit();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    float m_sliderValue = 0.0f;
    int m_testCounter = 0;
    bool m_bShowImguiDemo = false;
    bool m_bIsNative = false;
};

