#include "ViewInputMothod.h"


#include "../Game.h"
#include "../IMM/TsfInputMethodStore.h"
#include "ViewChat.h"

bool ViewInputMothod::OnInit() {
    return true;
}

void ViewInputMothod::OnRender() {

	static std::once_flag flag1;
	std::call_once(flag1, []() {
		auto chatWindowSize = ImVec2(300.0f, 300.0f);
		ImGui::SetNextWindowSize(chatWindowSize);
	});
	ImGui::Begin("input method:", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav);
	

	auto& tsf = TsfInputMethodStore::get();
	
	auto pChat = m_pChat.lock();
	
	std::wstring reading = pChat ? pChat->reading_buffer().c_str() : L"";
	do
	{
		if (reading.empty()) {
			reading += L"|";
			break;
		}
		if (tsf.candidate_select_start() >= reading.length()) {
			reading.append(L"|");
			break;
		}

		reading = reading.substr(0, tsf.candidate_select_start()) + L"|" + reading.substr(tsf.candidate_select_start());
	} while (false);

	ImGui::Text(tstring_to_string(reading).c_str());
	ImGui::Separator();
	for (std::size_t i = 0; i < tsf.candidates().size(); ++i)
	{
		auto candidate = std::to_wstring(i + 1) + L" " + tsf.candidates()[i];

		auto pageindex = tsf.candidate_select_index() % std::max((std::size_t)tsf.candidate_page_max_size(), tsf.candidates().size());
		if (i == pageindex) {
			candidate = (L"->") + candidate;
		}
		else {
			candidate = (L"  ") + candidate;
		}
		ImGui::Text(tstring_to_string(candidate).c_str());
	}

	ImGui::Text("%d/%d", tsf.candidate_page_index() + 1, tsf.candidate_page_count());


	ImGui::SameLine();
	ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
		"index: %d, total: %d, pagesize: %d",
		tsf.candidate_select_index() + 1,
		tsf.candidate_count(),
		tsf.candidate_page_size());

	ImGui::Text("%s: %s", TsfInputMethodStore::get().lang_id() == 0x409 ? "EN" : "ZH", 
		TsfInputMethodStore::get().ime_desc_a().c_str());
	ImGui::End();
}

void ViewInputMothod::OnDestroy() {

}