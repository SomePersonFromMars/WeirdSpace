#include "global_settings_gui.hpp"
#include "imgui.h"
#include "settings.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <utility>
using namespace glm;

#include "useful.hpp"

void global_settings_gui::draw_global_settings_controls() {
	if (ImGui::CollapsingHeader("Global settings controls")) {
		ImGui::DragScalar("font_global_scale", ImGuiDataType_Float,
			&global_settings.font_global_scale,
			0.1f,
			&global_settings.font_global_scale_min,
			&global_settings.font_global_scale_max);

		if (ImGui::Button("Save"))
			global_settings.save_settings_to_file();
        ImGui::SameLine();
		if (ImGui::Button("Soft load settings"))
			global_settings.load_settings_from_file();
        ImGui::SameLine();
		if (ImGui::Button("Load & reload application"))
			global_settings.request_global_reload();

        ImGui::Text("Predefined maps/settings:");
        ImGui::Text("Best view in GAME ITSELF:");
        {
            const char * const configs_names[] {
                "desert1.txt",
                "desert2.txt",
                "huge_mountains1.txt",
                "huge_mountains2.txt",
                "random_desert.txt",
            };
            for (std::size_t i = 0; i < ARR_SIZE(configs_names); ++i) {
                if (ImGui::Button(configs_names[i])) {
                    global_settings.load_settings_from_file((std::string("runtime/predefined_maps/") + std::string(configs_names[i])).c_str());
                    global_settings.request_possibly_no_restart_reload();
                }
            }
        }

        ImGui::Text("Best view in GENERATOR PLAYGROUND:");
        {
            const char * const configs_names[] {
                "evolving_GPU_continents1.txt",
                "evolving_GPU_continents2.txt",

                "big_continent_with_rivers1.txt",
                "big_continent_with_rivers2.txt",
                "continents_with_rivers1.txt",
                "continents_with_rivers2.txt",

                "random_continents_with_rivers_and_humidity.txt",
                "random_continents_with_rivers_and_temperature.txt",
                "random_GPU_continents.txt",
            };
            for (std::size_t i = 0; i < ARR_SIZE(configs_names); ++i) {
                if (ImGui::Button(configs_names[i])) {
                    global_settings.load_settings_from_file((std::string("runtime/predefined_maps/") + std::string(configs_names[i])).c_str());
                    global_settings.request_possibly_no_restart_reload();
                }
            }
        }
	}
}

void global_settings_gui::draw_general_settings() {
	if (ImGui::TreeNode(
				"General" /* , ImGuiTreeNodeFlags_DefaultOpen */)) {
		// constexpr std::size_t size_t_step = 1;
		// ImGui::InputScalar("debug_vals[0]", ImGuiDataType_U64,
		// 	&global_settings.debug_vals[0], &size_t_step);
		// ImGui::InputScalar("debug_vals[1]", ImGuiDataType_U64,
		// 	&global_settings.debug_vals[1], &size_t_step);
		// ImGui::InputScalar("debug_vals[2]", ImGuiDataType_U64,
		// 	&global_settings.debug_vals[2], &size_t_step);

		ImGui::DragScalar("map_unit_resolution", ImGuiDataType_S32,
			&global_settings.map_unit_resolution,
			1.0f,
			&global_settings.map_unit_resolution_min,
			&global_settings.map_unit_resolution_max);

		ImGui::DragScalar("map_width_in_units", ImGuiDataType_S32,
			&global_settings.map_width_in_units,
			1.0f,
			&global_settings.map_width_in_units_min,
			&global_settings.map_width_in_units_max);

		ImGui::DragScalar("map_height_in_units", ImGuiDataType_S32,
			&global_settings.map_height_in_units,
			1.0f,
			&global_settings.map_height_in_units_min,
			&global_settings.map_height_in_units_max);

		ImGui::Checkbox("generate_with_gpu",
			&global_settings.generate_with_gpu);

		if (global_settings.generate_with_gpu)
			ImGui::Checkbox("triple_map_size",
				&global_settings.triple_map_size);
		else
			global_settings.triple_map_size = true;

		ImGui::DragScalar("voro_cnt", ImGuiDataType_U64,
			&global_settings.voro_cnt,
			1.0f,
			&global_settings.voro_cnt_min,
			&global_settings.voro_cnt_max);

		ImGui::DragScalar("super_voro_cnt", ImGuiDataType_U64,
			&global_settings.super_voro_cnt,
			1.0f,
			&global_settings.super_voro_cnt_min,
			&global_settings.super_voro_cnt_max);

		ImGui::DragScalar("land_probability", ImGuiDataType_Float,
			&global_settings.land_probability,
			0.01f,
			&global_settings.land_probability_min,
			&global_settings.land_probability_max);

		ImGui::Checkbox("draw_mid_polygons",
			&global_settings.draw_mid_polygons);

		ImGui::Checkbox("enable_breakpoints",
			&enable_breakpoints);

		ImGui::DragScalar("replace_seed", ImGuiDataType_U64,
			&global_settings.replace_seed,
			1.0f,
			&global_settings.replace_seed_min,
			&global_settings.replace_seed_max);

        if (ImGui::Button("Fill in current seed")) {
            global_settings.request_replace_seed_overwrite();
        }

		ImGui::Checkbox("draw_player",
			&global_settings.draw_player);

		ImGui::Checkbox("dynamic_map",
			&global_settings.dynamic_map);

        ImGui::TreePop();
	}
}

void global_settings_gui::draw_rivers_and_climate_settings() {
	if (ImGui::TreeNode("Rivers and climate")) {
		ImGui::DragScalar("river_joints_R", ImGuiDataType_Double,
			&global_settings.river_joints_R,
			0.0002f,
			&global_settings.river_joints_R_min,
			&global_settings.river_joints_R_max);

		ImGui::DragScalar("river_start_prob", ImGuiDataType_S32,
			&global_settings.river_start_prob,
			1.0f,
			&global_settings.river_start_prob_min,
			&global_settings.river_start_prob_max);

		ImGui::DragScalar("river_branch_prob", ImGuiDataType_S32,
			&global_settings.river_branch_prob,
			1.0f,
			&global_settings.river_branch_prob_min,
			&global_settings.river_branch_prob_max);

		{
			static vec3 river_color_f
				= color_hex_to_vec3(global_settings.river_color);
			if (ImGui::ColorEdit3("river_color",
						(float*)&river_color_f))
				global_settings.river_color
					= color_vec3_to_hex(river_color_f);
		}

		ImGui::Checkbox("generate_rivers",
			&global_settings.generate_rivers);

		ImGui::Checkbox("draw_temperature",
			&global_settings.draw_temperature);

		ImGui::Checkbox("draw_humidity",
			&global_settings.draw_humidity);

		ImGui::DragScalar("humidity_scale", ImGuiDataType_S32,
			&global_settings.humidity_scale,
			1.0f,
			&global_settings.humidity_scale_min,
			&global_settings.humidity_scale_max);

		ImGui::DragScalar("temperature_exp", ImGuiDataType_Double,
			&global_settings.temperature_exp,
			0.0002f,
			&global_settings.temperature_exp_min,
			&global_settings.temperature_exp_max);
        ImGui::TreePop();
	}
}

void global_settings_gui::draw_imgui_widgets() {
	ImGui::SeparatorText("Global settings");

// #ifdef DEBUG
//     draw_global_settings_controls();
//     draw_general_settings();
//     draw_rivers_and_climate_settings();
// #endif

// #ifndef DEBUG
    draw_global_settings_controls();
	if (ImGui::CollapsingHeader("Advanced - global settings", ImGuiTreeNodeFlags_Bullet)) {
        draw_general_settings();
        draw_rivers_and_climate_settings();
    }
// #endif
}
