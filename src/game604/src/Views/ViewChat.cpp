#include "ViewChat.h"
#include "../Game.h"

bool ViewChat::OnInit() {
	m_strChatContent.resize(MAX_BUFFER_LENGTH);
	return true;
}

int GetUtf8Length(char const* str, int end) {
	int len = 0, index = 0;
	while (*str != '\0' && index < end) {
		len++;
		if ((*str & 0x80) == 0) {
			str++;
			index++;
		}
		else if ((*str & 0xF0) == 0xF0) {
			str += 4;
			index += 4;
		}
		else if ((*str & 0xE0) == 0xE0) {
			str += 3;
			index += 3;
		}
		else if ((*str & 0xC0) == 0xC0) {
			str += 2;
			index += 2;
		}
		else {
			return 0;
		}
	}
	return len;
}

int WStringLenToUtf8Length(char const* str, int wend) {
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
		else {
			return 0;
		}
	}
	return len;
}

int ViewChat::InputCallback(ImGuiInputTextCallbackData* data) {
	ViewChat* pViewChat = ((ViewChat*)(data->UserData));
	pViewChat->m_contentData = *data;
	if (!TsfInputMethodStore::get().is_reading_upate()) {
		data->BufDirty = false;
		return 0;
	}

	std::string s(wstring_to_utf8(std::wstring(pViewChat->m_strReadingBuffer.c_str())));
	// if (!s.empty()) 
	try
	{
		s.copy(data->Buf, std::size_t(data->BufSize) - 1);
		data->Buf[s.length()] = '\0';
		data->BufTextLen = strlen(data->Buf);
		data->BufDirty = true;
		data->CursorPos = WStringLenToUtf8Length(s.c_str(), TsfInputMethodStore::get().candidate_select_end());

		
		LOG_DEBUG("callback: {}, textlen: {}, cursor: {}, s:{}, s.len: {}", data->Buf, data->BufTextLen, data->CursorPos, s, s.length());
	}
	catch (...) {
		STM_DEBUG() << "exception";
	}
	// LOG_DEBUG("data: {}, {}, {} pos: {}", data->Buf, data->SelectionEnd, data->SelectionEnd, data->CursorPos);
	return 0;
}



void ViewChat::OnRender() {
	auto windowSize = ImGui::GetWindowSize();
	static float windowHight = 230.0f;
	static float historyWindowHight = 200.0f;
	auto chatWindowSize = ImVec2(400.0f, windowHight);
	ImGui::SetNextWindowPos(ImVec2(0.0f, Game::Inst().height() - chatWindowSize.y));
	ImGui::SetNextWindowSize(chatWindowSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, false);
	ImGui::Begin("Chat View", &m_isOpenChat, ImGuiWindowFlags_NoDecoration);                          // Create a window called "Hello, world!" and append into it.
	{
		if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
		{
			std::list<std::string>* plist = &m_worldHistories;
			if (ImGui::BeginTabItem("世界")) {
				plist = &m_worldHistories;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("公会")) {
				plist = &m_clanHistories;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("私聊")) {
				plist = &m_chatHistories;
				ImGui::EndTabItem();
			}

			ImGui::BeginChild("Chat History", ImVec2(0, -(windowHight - historyWindowHight)), true, ImGuiWindowFlags_None);
			for (const auto& h : *plist)
			{
				ImGui::Text(h.c_str());
			}
			ImGui::EndChild();

			ImGui::Spacing();

			auto windowSize = ImGui::GetWindowSize();
			ImGui::EndTabBar();
		}
		std::list<std::string>* plist = &m_worldHistories;
		ImGui::PushItemWidth(windowSize.x - 60);

		
		bool inputText = false;
		if (ImGui::InputText("##ChatInput", m_strChatContent.data(), m_strChatContent.length(), ImGuiInputTextFlags_CallbackAlways, &InputCallback, this)) {
			ImGui::SetItemDefaultFocus();
			inputText = true;
			LOG_DEBUG("after input, {}", m_strChatContent.c_str());
		}
		bool isActivie = false;
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow) && ImGui::IsItemActive()) {
			isActivie = true;
		}
		else {
			isActivie = false;
		}

		std::string_view content(m_contentData.Buf, m_contentData.BufTextLen);
		if (m_iLastContentPos != m_contentData.CursorPos || content != m_strLastContent) 
		{
			m_strLastContent = content;
			m_iLastContentPos = m_contentData.CursorPos;
			if (inputText == true) {
				// 内容改变了，有可能是删除操作
				std::wstring newcontent = utf8_to_wstring(content.data());
				if (newcontent != m_strReadingBuffer.c_str()) 
				{
					memset((void*)m_strReadingBuffer.c_str(), 0, m_strReadingBuffer.size());
					newcontent.copy(m_strReadingBuffer.data(), m_strReadingBuffer.length(), 0);
					TsfInputMethodStore::get().SetBufferLength(newcontent.length());
					LOG_DEBUG("content: {}, new: {}， buflen: {}", content, wstring_to_utf8(newcontent), m_contentData.BufTextLen);
				}
			}
			
			
			TsfInputMethodStore::get().SetCorsorPos(GetUtf8Length(content.data(), (int)m_iLastContentPos));
		}
		if (isActivie != m_lastIsFocus) {
			TsfInputMethodStore::get().SetFocus(isActivie);
		}
		
		m_lastIsFocus = isActivie;

		ImGui::PopItemWidth();
		//bool ret = false;
		//ImGui::BulletText(
		//	"Return value = %d\n"
		//	"IsItemFocused() = %d\n"
		//	"IsItemHovered() = %d\n"
		//	"IsItemHovered(_AllowWhenBlockedByPopup) = %d\n"
		//	"IsItemHovered(_AllowWhenBlockedByActiveItem) = %d\n"
		//	"IsItemHovered(_AllowWhenOverlapped) = %d\n"
		//	"IsItemHovered(_RectOnly) = %d\n"
		//	"IsItemActive() = %d\n"
		//	"IsItemEdited() = %d\n"
		//	"IsItemActivated() = %d\n"
		//	"IsItemDeactivated() = %d\n"
		//	"IsItemDeactivatedAfterEdit() = %d\n"
		//	"IsItemVisible() = %d\n"
		//	"IsItemClicked() = %d\n"
		//	"IsItemToggledOpen() = %d\n"
		//	"GetItemRectMin() = (%.1f, %.1f)\n"
		//	"GetItemRectMax() = (%.1f, %.1f)\n"
		//	"GetItemRectSize() = (%.1f, %.1f)",
		//	ret,
		//	ImGui::IsItemFocused(),
		//	ImGui::IsItemHovered(),
		//	ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
		//	ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
		//	ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped),
		//	ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly),
		//	ImGui::IsItemActive(),
		//	ImGui::IsItemEdited(),
		//	ImGui::IsItemActivated(),
		//	ImGui::IsItemDeactivated(),
		//	ImGui::IsItemDeactivatedAfterEdit(),
		//	ImGui::IsItemVisible(),
		//	ImGui::IsItemClicked(),
		//	ImGui::IsItemToggledOpen(),
		//	ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y,
		//	ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y,
		//	ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y
		//);

		//static bool embed_all_inside_a_child_window = false;
		//ImGui::Checkbox("Embed everything inside a child window (for additional testing)", &embed_all_inside_a_child_window);
		//if (embed_all_inside_a_child_window)
		//	ImGui::BeginChild("outer_child", ImVec2(0, ImGui::GetFontSize() * 20.0f), true);

		//// Testing IsWindowFocused() function with its various flags.
		//// Note that the ImGuiFocusedFlags_XXX flags can be combined.
		//ImGui::BulletText(
		//	"IsWindowFocused() = %d\n"
		//	"IsWindowFocused(_ChildWindows) = %d\n"
		//	"IsWindowFocused(_ChildWindows|_RootWindow) = %d\n"
		//	"IsWindowFocused(_RootWindow) = %d\n"
		//	"IsWindowFocused(_AnyWindow) = %d\n",
		//	ImGui::IsWindowFocused(),
		//	ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows),
		//	ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootWindow),
		//	ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow),
		//	ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow));

		//// Testing IsWindowHovered() function with its various flags.
		//// Note that the ImGuiHoveredFlags_XXX flags can be combined.
		//ImGui::BulletText(
		//	"IsWindowHovered() = %d\n"
		//	"IsWindowHovered(_AllowWhenBlockedByPopup) = %d\n"
		//	"IsWindowHovered(_AllowWhenBlockedByActiveItem) = %d\n"
		//	"IsWindowHovered(_ChildWindows) = %d\n"
		//	"IsWindowHovered(_ChildWindows|_RootWindow) = %d\n"
		//	"IsWindowHovered(_ChildWindows|_AllowWhenBlockedByPopup) = %d\n"
		//	"IsWindowHovered(_RootWindow) = %d\n"
		//	"IsWindowHovered(_AnyWindow) = %d\n",
		//	ImGui::IsWindowHovered(),
		//	ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
		//	ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
		//	ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows),
		//	ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow),
		//	ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup),
		//	ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow),
		//	ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
		ImGui::SameLine();
		if (ImGui::Button("Send")) {
			(*plist).emplace_back(m_strChatContent);
			
			m_strChatContent.clear();
			m_strChatContent.resize(1024);
			m_strReadingBuffer.clear();
			m_strReadingBuffer.resize(1024);
			TsfInputMethodStore::get().SetBufferLength(0);
		};
		
	}
	ImGui::End();
	ImGui::PopStyleVar();
}
