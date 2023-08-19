#include "app.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <useful.hpp>
#include <imgui_basic_controls.hpp>
#include <global_settings_gui.hpp>

void app_t::init_imgui() {
	imgui_basic_controls::init_imgui(window);
}

void app_t::draw_playground_specific_imgui_widgets() {
	ImGui::SeparatorText("Playground specific");

	if (ImGui::CollapsingHeader("Generator controls")) {
		if (ImGui::Button("Reload"))
			reload_procedure();
		if (ImGui::Button("Soft reload"))
			soft_reload_procedure();
	}
	if (ImGui::CollapsingHeader("Outputs")) {
		ImGui::Text("mp: {%f, %f}", mp.x, mp.y);
		ImGui::Text("win: {%d, %d}",
			window_width, window_height);
	}
}

void app_t::in_loop_update_imgui() {
	imgui_basic_controls::begin_drawing();

	imgui_basic_controls::draw_demo_windows();
	global_settings_gui::draw_imgui_widgets();
	draw_playground_specific_imgui_widgets();

	imgui_basic_controls::end_drawing();
}

void app_t::deinit_imgui() {
	imgui_basic_controls::deinit_imgui();
}
