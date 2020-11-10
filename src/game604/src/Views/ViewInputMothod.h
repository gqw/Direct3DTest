#pragma once

#include "../pch.h"
#include "../ViewInterface.h"

class ViewChat;
class ViewInputMothod : public ViewInterface
{
public:
    ViewInputMothod(std::string_view name, std::shared_ptr<ViewChat> pChat) : ViewInterface(name), m_pChat(pChat) {};
    virtual ~ViewInputMothod() {}

    virtual bool OnInit();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    float m_sliderValue = 0.0f;
    int m_testCounter = 0;
    bool m_lastIsFocus = false;
    std::weak_ptr<ViewChat> m_pChat;
};

