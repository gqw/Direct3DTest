#pragma once

#include "../pch.h"
#include "../ViewInterface.h"

class ViewChat : public ViewInterface
{
    static constexpr uint32_t MAX_BUFFER_LENGTH = 1024;
public:
    ViewChat(std::string_view name) : ViewInterface(name) { m_strReadingBuffer.resize(MAX_BUFFER_LENGTH); };
    virtual ~ViewChat() {}
    virtual bool OnInit();
    virtual void OnRender();

    std::wstring& reading_buffer() { return m_strReadingBuffer; };

private:
    static int InputCallback(ImGuiInputTextCallbackData* data);

private:
    std::string m_strChatContent;

    std::list<std::string> m_worldHistories;
    std::list<std::string> m_clanHistories;
    std::list<std::string> m_chatHistories;
    bool m_isOpenChat = true;
    bool m_lastIsFocus = false;
    ImGuiInputTextCallbackData m_contentData;

    std::wstring m_strReadingBuffer;
    std::wstring m_strLastReading;
    std::string m_strLastContent;
    std::size_t m_iLastContentPos = 0;
};

