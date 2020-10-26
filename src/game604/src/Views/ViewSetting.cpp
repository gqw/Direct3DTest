#include "ViewSetting.h"

#include "../Game.h"

bool ViewSetting::OnInit() {
    return true;
}

void ViewSetting::OnRender() {
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::Begin("Setting Page"); 
	{
		ImGui::Text("Options:");  


		if (ImGui::Button("Fullscreen")) {
			Game::Inst().OnChangeFullscreen();
		}
		ImGui::SameLine();
		if (ImGui::Button("Switch IME")) {
			TsfInputMethodStore::get().SwitchImeConvert();
		}

		ImGui::Checkbox("Show Imgui Demo", &m_bShowImguiDemo);
		ImGui::Text("Application average frame (%.1f FPS)", ImGui::GetIO().Framerate);
	}
	ImGui::End();

	if (m_bShowImguiDemo) ImGui::ShowDemoWindow(&m_bShowImguiDemo);
}

void ViewSetting::OnDestroy() {

}