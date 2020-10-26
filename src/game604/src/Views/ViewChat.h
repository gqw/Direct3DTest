#pragma once

#include "../pch.h"
#include "../ViewInterface.h"

class ViewChat : public ViewInterface
{
public:
    ViewChat(std::string_view name) : ViewInterface(name) {};
    virtual ~ViewChat() {}
    virtual bool OnInit();
    virtual void OnRender();

private:
    std::string m_strChatContent;

    std::list<std::string> m_worldHistories;
    std::list<std::string> m_clanHistories;
    std::list<std::string> m_chatHistories;
    bool m_isOpenChat = true;
    bool m_lastIsFocus = false;
};

