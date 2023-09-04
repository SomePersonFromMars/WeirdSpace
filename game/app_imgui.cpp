#include "app.hpp"

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

	imgui_basic_controls::draw_demo_windows();
	draw_game_specific_imgui_widgets();
	global_settings_gui::draw_imgui_widgets();

	imgui_basic_controls::end_drawing();
}

void app_t::draw_game_specific_imgui_widgets() {
	ImGui::SeparatorText("Game specific");

	if (ImGui::CollapsingHeader("Chunks rendering")) {
		// const ImGuiTableFlags flags = ImGuiTableFlags_Borders;
		// const ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
		const ImGuiTableFlags flags = 0;
		if (ImGui::BeginTable(
				"Rendered chunks table", world_buffer.width, flags)) {
			// constexpr ImU32 GREEN = rgba_to_abgr(0x44c457ffu);
			// constexpr ImU32 GREEN = rgba_to_abgr(0x26592effu);
			constexpr ImU32 GREEN = rgba_to_abgr(0x3e6344ffu);
			// constexpr ImU32 RED = rgba_to_abgr(0x63423effu);
			constexpr ImU32 RED = rgba_to_abgr(0x935555ffu);

			for (int z = 0; z < world_buffer.depth; z++) {
				ImGui::TableNextRow();
				for (int x = 0; x < world_buffer.width; x++) {
					ImGui::TableSetColumnIndex(x);
					const auto chunk_it = world_buffer.chunks.find({x, z});
					if (chunk_it != world_buffer.chunks.end()) {
						chunk_t &chunk = chunk_it->second;
						ImGui::TableSetBgColor(
							ImGuiTableBgTarget_CellBg,
							chunk.is_rendering_enabled() ? GREEN : RED);

						ImGui::PushID(z * world_buffer.width + x);
						if (ImGui::Button("..")) {
							// chunk.enable_rendering(
							// 	not chunk.is_rendering_enabled());
						}
						ImGui::PopID();
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
		const double FPS = 1.0 / delta_time;
		ImGui::Text("FPS: %f", FPS);
		ImGui::Text("window dimensions: {%d, %d}",
			window_width, window_height);
	}
}
