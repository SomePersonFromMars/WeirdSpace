#include "global_settings_gui.hpp"
#include "imgui_basic_controls.hpp"
#include "settings.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "useful.hpp"

void global_settings_gui::draw_imgui_widgets() {
	ImGui::SeparatorText("Global settings");

	if (ImGui::CollapsingHeader("Global settings controls")) {
		if (ImGui::Button("Save"))
			global_settings.save_settings_to_file();
		if (ImGui::Button("Load"))
			global_settings.load_settings_from_file();
	}

	if (ImGui::CollapsingHeader(
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

		ImGui::Checkbox("draw_mid_polygons",
			&global_settings.draw_mid_polygons);

		ImGui::Checkbox("enable_breakpoints",
			&enable_breakpoints);

		ImGui::DragScalar("replace_seed", ImGuiDataType_U64,
			&global_settings.replace_seed,
			1.0f,
			&global_settings.replace_seed_min,
			&global_settings.replace_seed_max);

		ImGui::Checkbox("draw_player",
			&global_settings.draw_player);
	}

	if (ImGui::CollapsingHeader("Rivers and climate")) {
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
	}
}
