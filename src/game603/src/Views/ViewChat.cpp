#include "ViewChat.h"
#include "../Game.h"

bool ViewChat::OnInit() {
	m_strChatContent.resize(1024);
	return true;
}

void ViewChat::OnRender() {
	auto windowSize = ImGui::GetWindowSize();
	auto chatWindowSize = ImVec2(400.0f, 200.0f);
	ImGui::SetNextWindowPos(ImVec2(0.0f, Game::Inst().height() - chatWindowSize.y));
	ImGui::SetNextWindowSize(chatWindowSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, false);
	ImGui::Begin("Chat View", nullptr, ImGuiWindowFlags_NoDecoration);                          // Create a window called "Hello, world!" and append into it.
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

			ImGui::BeginChild("Chat History", ImVec2(0, -60), true, ImGuiWindowFlags_None);
			for (const auto& h : *plist)
			{
				ImGui::Text(h.c_str());
			}
			ImGui::EndChild();

			ImGui::Spacing();

			auto windowSize = ImGui::GetWindowSize();
			auto itemWindowSize = ImGui::GetItemRectSize();

			ImGui::PushItemWidth(windowSize.x - 60);
			ImGui::InputText("aa", m_strChatContent.data(), m_strChatContent.length());
			ImGui::PopItemWidth();
			ImGui::PushItemWidth(windowSize.x - 60);
			ImGui::InputText("bb", m_strChatContent.data(), m_strChatContent.length());
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::Button("Send")) {
				(*plist).emplace_back(m_strChatContent);
				m_strChatContent.clear();
				m_strChatContent.resize(1024);
			};
			ImGui::EndTabBar();
		}
		
	}
	ImGui::End();
	ImGui::PopStyleVar();
}
