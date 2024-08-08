// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "app.hpp"
#include "imgui.h"

#include <chrono>
#include <useful.hpp>
#include <settings.hpp>
#include <imgui_basic_controls.hpp>
#include <global_settings_gui.hpp>
using namespace glm;

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
    if (ImGui::Begin("Instructions, More Worlds & Settings")) {
        ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);

#ifndef DEBUG
        draw_game_instructions();
#endif
        global_settings_gui::draw_imgui_widgets();
        draw_game_specific_imgui_widgets();
    }
	ImGui::End();

	imgui_basic_controls::end_drawing();
}

void app_t::draw_game_settings() {
	if (ImGui::CollapsingHeader(
            "Settings"
#ifndef DEBUG
            , ImGuiTreeNodeFlags_DefaultOpen
#endif
            )) {
        ImGui::Text("Atmosphere colors:");
        ImGui::SameLine();
        if (ImGui::SmallButton(" 1 ")) {
            global_settings.sky_color = 0xd7e6e8;
            global_settings.light_color = 0xfce7b5;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(" 2 ")) {
            global_settings.sky_color = 0xc9edf2;
            global_settings.light_color = 0xffffff;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(" 3 ")) {
            global_settings.sky_color = 0xfcb967;
            global_settings.light_color = 0xf7d5ad;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(" 4 ")) {
            global_settings.sky_color = 0xfccc92;
            global_settings.light_color = 0xf7d5ad;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(" 5 ")) {
            global_settings.sky_color = 0x000000;
            global_settings.light_color = 0xffffff;
        }

        {
            vec3 sky_color_f
                = color_hex_to_vec3(global_settings.sky_color);
            if (ImGui::ColorEdit3("sky_color",
                        (float*)&sky_color_f))
                global_settings.sky_color
                    = color_vec3_to_hex(sky_color_f);
        }
        {
            vec3 light_color_f
                = color_hex_to_vec3(global_settings.light_color);
            if (ImGui::ColorEdit3("light_color",
                        (float*)&light_color_f))
                global_settings.light_color
                    = color_vec3_to_hex(light_color_f);
        }
	}
}

void app_t::draw_outputs() {
	if (ImGui::CollapsingHeader(
            "Outputs" //,
            // ImGuiTreeNodeFlags_DefaultOpen
            )) {
        const auto now = std::chrono::high_resolution_clock::now();
        static auto last_update = now;
		static double fps_cnt = 1.0 / delta_time;
        using namespace std::chrono_literals;
        if ((now - last_update) >= 100ms) {
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

void app_t::draw_chunks_info() {
    [[maybe_unused]] constexpr ImU32 GREEN = rgba_to_abgr(0x3e6344ffu);
    [[maybe_unused]] constexpr ImU32 RED = rgba_to_abgr(0x935555ffu);

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	if (ImGui::TreeNode(
            "Chunks preprocessing priority" //,
            // ImGuiTreeNodeFlags_DefaultOpen
            // ImGuiTreeNodeFlags_None
            )) {

		const ImGuiTableFlags flags = 0;
		if (ImGui::BeginTable(
				"Chunks preprocessing priority", world_buffer.get_buffer_width(), flags)) {

			for (int z = 0; z < world_buffer.get_buffer_depth(); z++) {
				ImGui::TableNextRow();
				for (int x = 0; x < world_buffer.get_buffer_width(); x++) {
                    const glm::ivec2 buffer_pos_XZ = {x, z};

					ImGui::TableSetColumnIndex(x);
                    const chunk_info_t * const chunk_info_ptr = world_buffer.get_chunk_info_ptr(buffer_pos_XZ);
					// const auto chunk_info_it = world_buffer.chunks_info.find(buffer_pos_XZ);
					if (chunk_info_ptr != nullptr) {
						const chunk_info_t &chunk_info = *chunk_info_ptr;
                        // const chunk_rendering_t &chunk_rendering = world_buffer.preprocessed_chunks_buffer[chunk_info.id_in_expiration_queue].first;

                        const float level = chunk_info.preprocessing_priority;
                        const ImU32 color = rgba_to_abgr((hsv_to_rgb(level * 120.0f / 360.0f, 0.4f, 0.7f) << 8) | 0xff);
                        // const ImU32 color = chunk_rendering.is_rendering_enabled() ? GREEN : RED;

						ImGui::TableSetBgColor(
							ImGuiTableBgTarget_CellBg,
							color);
                        // ImGui::Text(
                        //     "%.2f",
                        //     static_cast<double>(level));
                        ImGui::Text(
                            "%zu",
                            chunk_info.id_in_expiration_queue);
					}
				}
			}
			ImGui::EndTable();
		}
        ImGui::TreePop();
	}
}

void app_t::draw_game_instructions() {
	ImGui::SeparatorText("Instructions");
	ImGui::Text("Keybindings:");
    ImGui::BulletText("Player movement:");
    ImGui::Indent();
    ImGui::BulletText("Use ARROW KEYS to move up/left/down/right.");
    ImGui::BulletText("Use SPACE to jump.");
    ImGui::BulletText("Use F to switch flying.");
    ImGui::BulletText("Hold Z to accelerate movement.");
    ImGui::BulletText("Use R to return to the starting position.");
    ImGui::Unindent();
    ImGui::BulletText("Camera movement:");
    ImGui::Indent();
    ImGui::BulletText("Use G to switch player following.");
    ImGui::BulletText("Use W/A/S/D to move.");
    ImGui::BulletText("Use I/J/K/L or NUMPAD to rotate.");
    ImGui::BulletText("Hold L-SHIFT or SEMICOLON (;) to accelerate movement.");
    ImGui::Unindent();

    ImGui::NewLine();
}

void app_t::draw_game_specific_imgui_widgets() {
	ImGui::SeparatorText("Game specific");

// #ifdef DEBUG
//     draw_game_settings();
//     draw_outputs();
//     draw_chunks_info();
// #endif

// #ifndef DEBUG
    draw_game_settings();
    draw_outputs();

	if (ImGui::CollapsingHeader(
            "Advanced - game specific",
            ImGuiTreeNodeFlags_DefaultOpen |
            // ImGuiTreeNodeFlags_None
            ImGuiTreeNodeFlags_Bullet
            )) {
        draw_chunks_info();

		if (ImGui::SliderFloat("render_distance",
			&global_settings.render_distance,
			global_settings.render_distance_min,
			global_settings.render_distance_max)) {
            global_settings.mark_chunks_rendering_settings_updated();
            max_replace(
                global_settings.max_preprocessed_chunks_cnt,
                world_buffer.calculate_min_max_preprocessed_chunks_cnt_for_render_distance(global_settings.render_distance)
                );
        }

		if (ImGui::DragScalar("max_preprocessed_chunks_cnt", ImGuiDataType_U64,
			&global_settings.max_preprocessed_chunks_cnt,
			1.0f,
			&global_settings.max_preprocessed_chunks_cnt_min,
			&global_settings.max_preprocessed_chunks_cnt_max)) {
            global_settings.mark_chunks_rendering_settings_updated();
        }

		ImGui::DragScalar("terrain_height_in_blocks", ImGuiDataType_S32,
			&global_settings.terrain_height_in_blocks,
			1.0f,
			&global_settings.terrain_height_in_blocks_min,
			&global_settings.terrain_height_in_blocks_max);

		ImGui::SliderFloat("camera_moving_speed_normal",
			&global_settings.camera_moving_speed_normal,
			global_settings.camera_moving_speed_normal_min,
			global_settings.camera_moving_speed_normal_max);

		ImGui::SliderFloat("camera_rotation_speed_normal",
			&global_settings.camera_rotation_speed_normal,
			global_settings.camera_rotation_speed_normal_min,
			global_settings.camera_rotation_speed_normal_max);

        if (ImGui::Button("Move player to camera")) {
            move_player_to_camera();
        }
    }
// #endif
}

void app_t::move_player_to_camera() {
    glm::vec3 new_player_pos = camera.get_position();
    new_player_pos.y = global_settings.terrain_height_in_blocks;
    new_player_pos.z = std::floor(new_player_pos.z) + 0.5f;

    player.set_position(new_player_pos);
    global_settings.default_player_position[0] = new_player_pos[0];
    global_settings.default_player_position[1] = new_player_pos[1];
    global_settings.default_player_position[2] = new_player_pos[2];
}
