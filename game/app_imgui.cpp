#include "app.hpp"

#include <chrono>
#include <useful.hpp>
#include <settings.hpp>
#include <imgui_basic_controls.hpp>
#include <global_settings_gui.hpp>

void app_t::init_imgui() {
	imgui_basic_controls::init_imgui(window);
}

void app_t::deinit_imgui() {
	imgui_basic_controls::deinit_imgui();
}


void app_t::in_loop_update_imgui() {
	imgui_basic_controls::begin_drawing();

#ifdef DEBUG
	imgui_basic_controls::draw_demo_windows();
#endif
	draw_game_specific_imgui_widgets();
	global_settings_gui::draw_imgui_widgets();

	imgui_basic_controls::end_drawing();
}

void app_t::draw_game_specific_imgui_widgets() {
	ImGui::SeparatorText("Game specific");

	if (ImGui::CollapsingHeader("Chunks rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
		// const ImGuiTableFlags flags = ImGuiTableFlags_Borders;
		// const ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
		const ImGuiTableFlags flags = 0;
		if (ImGui::BeginTable(
				"Rendered chunks table", world_buffer.get_buffer_width(), flags)) {
			constexpr ImU32 GREEN = rgba_to_abgr(0x3e6344ffu);
			constexpr ImU32 RED = rgba_to_abgr(0x935555ffu);

			for (int z = 0; z < world_buffer.get_buffer_depth(); z++) {
				ImGui::TableNextRow();
				for (int x = 0; x < world_buffer.get_buffer_width(); x++) {
					ImGui::TableSetColumnIndex(x);
					const auto chunk_it = world_buffer.chunks.find({x, z});
					if (chunk_it != world_buffer.chunks.end()) {
						chunk_t &chunk = chunk_it->second;
						ImGui::TableSetBgColor(
							ImGuiTableBgTarget_CellBg,
							chunk.is_rendering_enabled() ? GREEN : RED);

                        ImGui::Text(".");
					}
				}
			}
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader("Chunks generate priority", ImGuiTreeNodeFlags_DefaultOpen)) {
		const ImGuiTableFlags flags = 0;
		if (ImGui::BeginTable(
				"Chunks generate priority", world_buffer.get_buffer_width(), flags)) {
			constexpr ImU32 GREEN = rgba_to_abgr(0x3e6344ffu);
			constexpr ImU32 RED = rgba_to_abgr(0x935555ffu);

			for (int z = 0; z < world_buffer.get_buffer_depth(); z++) {
				ImGui::TableNextRow();
				for (int x = 0; x < world_buffer.get_buffer_width(); x++) {
					ImGui::TableSetColumnIndex(x);
					const auto chunk_it = world_buffer.chunks.find({x, z});
					if (chunk_it != world_buffer.chunks.end()) {
						chunk_t &chunk = chunk_it->second;

                        const float level = chunk.get_preprocessing_priority();
                        const ImU32 color = rgba_to_abgr((hsv_to_rgb(level * 120.0f / 360.0f, 0.4f, 0.7f) << 8) | 0xff);

						ImGui::TableSetBgColor(
							ImGuiTableBgTarget_CellBg,
							color);
                        ImGui::Text(
                            "%.2f",
                            static_cast<double>(level));
					}
				}
			}
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader("Settings")) {
		ImGui::DragScalar("max_preprocessed_chunks_cnt", ImGuiDataType_U64,
			&global_settings.max_preprocessed_chunks_cnt,
			1.0f,
			&global_settings.max_preprocessed_chunks_cnt_min,
			&global_settings.max_preprocessed_chunks_cnt_max);
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

		ImGui::Text("window dimensions: {%d, %d}",
			window_width, window_height);
		ImGui::Text("FPS: %f", fps_cnt);
        ImGui::Text("Camera:");
        ImGui::Text("pos=(%f, %f, %f)",
                camera.get_position().x,
                camera.get_position().y,
                camera.get_position().z);
        ImGui::Text("angle=(%f, %f)",
                camera.get_horizontal_rotation_angle(),
                camera.get_vertical_rotation_angle());
        ImGui::Text("Player:");
        ImGui::Text("pos=(%f, %f, %f)",
                player.get_position().x,
                player.get_position().y,
                player.get_position().z);
	}
}
