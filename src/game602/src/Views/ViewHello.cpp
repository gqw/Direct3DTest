#include "ViewHello.h"

#include "../Game.h"

bool ViewHello::OnInit() {
    return true;
}

void ViewHello::OnRender() {
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

	ImGui::SliderFloat("float", &m_sliderValue, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	// ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	if (ImGui::Button("IncCounter"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		m_testCounter++;
	ImGui::SameLine();

	ImGui::Text("counter = %d", m_testCounter);

	if (ImGui::Button("Fullscreen")) {
		Game::Inst().OnChangeFullscreen();
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}

void ViewHello::OnDestroy() {

}