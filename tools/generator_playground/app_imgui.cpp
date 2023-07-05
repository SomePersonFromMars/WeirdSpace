#include "app.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "useful.hpp"

void app_t::init_imgui() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Setup Dear ImGui style
	// ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();
	ImGui::GetIO().FontGlobalScale = 2.0f;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	glClearColor(background_color.x, background_color.y, background_color.z,
			0.0f);
}

void app_t::loop_imgui() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Debug window
	{
		if (ImGui::CollapsingHeader("Buttons")) {
			if (ImGui::Button("Save"))
				global_settings.save_settings_to_file();
			if (ImGui::Button("Load"))
				global_settings.load_settings_from_file();
			if (ImGui::Button("Reload"))
				reload_procedure();
			if (ImGui::Button("Soft reload"))
				soft_reload_procedure();
		}

		if (ImGui::CollapsingHeader(
					"General", ImGuiTreeNodeFlags_DefaultOpen)) {
			// constexpr std::size_t size_t_step = 1;
			// ImGui::InputScalar("debug_vals[0]", ImGuiDataType_U64,
			// 	&global_settings.debug_vals[0], &size_t_step);
			// ImGui::InputScalar("debug_vals[1]", ImGuiDataType_U64,
			// 	&global_settings.debug_vals[1], &size_t_step);
			// ImGui::InputScalar("debug_vals[2]", ImGuiDataType_U64,
			// 	&global_settings.debug_vals[2], &size_t_step);

			ImGui::DragScalar("chunk_dim", ImGuiDataType_S32,
				&global_settings.chunk_dim,
				1.0f,
				&global_settings.chunk_dim_min,
				&global_settings.chunk_dim_max);

			// if (not global_settings.triple_bitmap_size)
			// 	global_settings.generate_with_gpu = true;

			ImGui::Checkbox("generate_with_gpu",
				&global_settings.generate_with_gpu);

			if (global_settings.generate_with_gpu)
				ImGui::Checkbox("triple_bitmap_size",
					&global_settings.triple_bitmap_size);
			else
				global_settings.triple_bitmap_size = true;

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

		if (ImGui::CollapsingHeader("Outputs")) {
			ImGui::Text("mp: {%f, %f}", mp.x, mp.y);
			ImGui::Text("win: {%d, %d}",
				window_width, window_height);
		}
	}

	// 1. Show the big demo window
	// (Most of the sample code is in ImGui::ShowDemoWindow()! You can
	// browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves.
	// We use a Begin/End pair to create a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");

		ImGui::Text("This is some useful text.");
		ImGui::Checkbox("Demo Window", &show_demo_window);
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);

		if (ImGui::Button("Button"))
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
				1000.0f / ImGui::GetIO().Framerate,
				ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void app_t::deinit_imgui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
