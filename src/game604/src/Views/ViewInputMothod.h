#pragma once

#include "../pch.h"
#include "../ViewInterface.h"

class ViewInputMothod : public ViewInterface
{
public:
    ViewInputMothod(std::string_view name) : ViewInterface(name) {};
    virtual ~ViewInputMothod() {}

    virtual bool OnInit();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    float m_sliderValue = 0.0f;
    int m_testCounter = 0;
    bool m_lastIsFocus = false;
};

