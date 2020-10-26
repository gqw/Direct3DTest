#include "ViewChat.h"
#include "../Game.h"

bool ViewChat::OnInit() {
	m_strChatContent.resize(1024);
	return true;
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
		if (ImGui::InputText("##ChatInput", m_strChatContent.data(), m_strChatContent.length())) {
			ImGui::SetItemDefaultFocus();
		}
		bool isActivie = false;
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow) && ImGui::IsItemActive()) {
			isActivie = true;
		}
		else {
			isActivie = false;
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
		};
		
	}
	ImGui::End();
	ImGui::PopStyleVar();
}
