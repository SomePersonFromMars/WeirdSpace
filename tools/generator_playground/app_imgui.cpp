// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "app.hpp"
#include "imgui.h"

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
        ImGui::SameLine();
		if (ImGui::Button("Soft reload"))
			soft_reload_procedure();
	}
	if (ImGui::CollapsingHeader("Outputs")) {
        const auto now = std::chrono::high_resolution_clock::now();
        static auto last_update = now;
		static double fps_cnt = 1.0 / delta_time;
        using namespace std::chrono_literals;
        if ((now - last_update) >= 500ms) {
            last_update = now;
            fps_cnt = 1.0 / delta_time;
        }

		ImGui::Text("FPS: %f", fps_cnt);
		ImGui::Text("mp: {%f, %f}", mp.x, mp.y);
		ImGui::Text("win: {%d, %d}",
			window_width, window_height);
	}
}

void app_t::in_loop_update_imgui() {
	imgui_basic_controls::begin_drawing();

#ifdef DEBUG
	imgui_basic_controls::draw_demo_windows();
#endif
	global_settings_gui::draw_imgui_widgets();
	draw_playground_specific_imgui_widgets();

	imgui_basic_controls::end_drawing();
}

void app_t::deinit_imgui() {
	imgui_basic_controls::deinit_imgui();
}
