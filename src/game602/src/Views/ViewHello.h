#pragma once

#include "../pch.h"
#include "../ViewInterface.h"

class ViewHello : public ViewInterface
{
public:
    ViewHello(std::string_view name) : ViewInterface(name) {};
    virtual ~ViewHello() {}

    virtual bool OnInit();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    float m_sliderValue = 0.0f;
    int m_testCounter = 0;
};

