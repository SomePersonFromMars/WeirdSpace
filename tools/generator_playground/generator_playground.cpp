#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "useful.hpp"
#include "settings.hpp"

#include "bitmap.hpp"
#include "line.hpp"
#include "generator.hpp"

struct callbacks_strct_t {
	GLint window_width, window_height;
	generator_t *generator;
	bool refresh_required = false;

	static inline callbacks_strct_t* get_strct(GLFWwindow *window) {
		return reinterpret_cast<callbacks_strct_t*>(
			glfwGetWindowUserPointer(window)
		);
	}
};

void window_size_callback(GLFWwindow* window, int width, int height) {
	callbacks_strct_t * const strct_ptr
		= callbacks_strct_t::get_strct(window);

	strct_ptr->window_width = width;
	strct_ptr->window_height = height;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window,
		int key, int scancode, int action, int mods) {
	callbacks_strct_t * const strct_ptr
		= callbacks_strct_t::get_strct(window);

	if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {
		PRINT_ZU(++strct_ptr->generator->debug_vals[0]);
		strct_ptr->refresh_required = true;
	}
	if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
		if (strct_ptr->generator->debug_vals[0] != 0) {
			PRINT_ZU(--strct_ptr->generator->debug_vals[0]);
			strct_ptr->refresh_required = true;
		}
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		++strct_ptr->generator->debug_vals[1];
		strct_ptr->refresh_required = true;
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		if (strct_ptr->generator->debug_vals[1] != 0) {
			--strct_ptr->generator->debug_vals[1];
			strct_ptr->refresh_required = true;
		}
	}
}

void fun_bitmap(bitmap_t &bitmap);

int32_t main(void) {
	GLFWwindow* window;

	callbacks_strct_t callbacksk_strct;
	GLint &window_width = callbacksk_strct.window_width;
	GLint &window_height = callbacksk_strct.window_height;

	window_width = 1080*2;
	window_height = 1080;

	// window_width = 1080;
	// window_height = 1080+10;

	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow( window_width, window_height, "Playground",
			nullptr, nullptr);
	if( window == nullptr ){
		fprintf(stderr,
			"Failed to open GLFW window. If you have an Intel GPU, they"
			"are not 3.3 compatible.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
	glfwSetWindowUserPointer(window,
			reinterpret_cast<void*>(&callbacksk_strct));
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);

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

	// Imgui state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	global_settings.load_settings_from_file();

	const auto background_color = color_hex_to_vec3(0);
	glClearColor(background_color.x, background_color.y, background_color.z,
			0.0f);

	bitmap_t bitmapA;
	generator_A_t generator_A;
	generator_B_t generator_B;
	generator_C_t generator_C;
	generator_t * const generator = &generator_C;
	callbacksk_strct.generator = generator;
	int resolution_div = 8;
	generator_C.generate_bitmap(bitmapA, resolution_div);
	generator->generate_bitmap(bitmapA, resolution_div);
	bitmapA.load_to_texture();

	const auto soft_reload_procedure
		= [&generator, &bitmapA, &resolution_div]() {
		generator->generate_bitmap(bitmapA, resolution_div);
		bitmapA.load_to_texture();
	};

	const auto reload_procedure = [&generator, &bitmapA, &resolution_div]() {
		generator->load_settings();
		generator->new_seed();
		generator->generate_bitmap(bitmapA, resolution_div);
		bitmapA.load_to_texture();
	};

	line_t line;
	double line_off = 0;

	vec3 camera_pos(0, 0, 0);
	float camera_zoom(1);

	std::chrono::time_point<std::chrono::high_resolution_clock>
		timer_logging = std::chrono::high_resolution_clock::now();
	double timer_fps_cnter = glfwGetTime();
	double delta_time = 0.0;

	while (glfwWindowShouldClose(window) == GLFW_FALSE) {

		constexpr auto frame_min_duration
			= std::chrono::milliseconds(FRAME_MIN_DURATION);

		const auto frame_beg_time = std::chrono::high_resolution_clock::now();
		const auto frame_end_time
			= frame_beg_time + frame_min_duration;

		if (callbacksk_strct.refresh_required) {
			callbacksk_strct.refresh_required = false;
			soft_reload_procedure();
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			reload_procedure();
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			camera_pos = vec3(0);
			camera_zoom = 1;
		}

		if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
			if (resolution_div >= 0)
				resolution_div -= 1;
		}
		if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
			resolution_div += 1;
		}

		{
			const float move_off = delta_time * 0.5 / camera_zoom;
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
				camera_pos.x -= move_off;
			}
			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
				camera_pos.x += move_off;
			}
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
				camera_pos.y -= move_off;
			}
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
				camera_pos.y += move_off;
			}
		}
		{
			// const float move_off = delta_time * 0.5;
			const double move_off = delta_time * 4.0;
			if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
				// line_pos.x -= move_off;
				line_off -= move_off;
			}
			if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
				// line_pos.x += move_off;
				line_off += move_off;
			}
			// if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
			// 	line_pos.y -= move_off;
			// }
			// if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
			// 	line_pos.y += move_off;
			// }
		}

		// if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE) {
		// 	++generator->debug_vals[0];
		// 	PRINT_ZU(generator->debug_vals[0]);
		// }
		// if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE) {
		// 	--generator->debug_vals[0];
		// 	PRINT_ZU(generator->debug_vals[0]);
		// }

		const float zoom_off = delta_time * 1.0;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			camera_zoom *= 1+zoom_off;
			// generator->generate_bitmap(bitmapA, resolution_div);
			// bitmapA.load_to_texture();
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			camera_zoom *= 1-zoom_off;
			// generator->generate_bitmap(bitmapA, resolution_div);
			// bitmapA.load_to_texture();
		}

		// Calculate MVP for bitmap
		mat4 MVPb(1);
		{
			MVPb = scale(MVPb, vec3(camera_zoom));
			MVPb = scale(
				MVPb, vec3(1, float(window_width)/float(window_height), 1));

			MVPb = translate(MVPb, -camera_pos *
					vec3(1, float(window_width)/float(window_height), 1)
				);

			MVPb = scale(MVPb,
					vec3(1, float(bitmapA.HEIGHT)/float(bitmapA.WIDTH), 1));
		}

		// Calculate pixel pointed by mouse cursor
		vec3 mp;
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			xpos /= (double)window_width;
			ypos /= (double)window_height;
			// ypos = 1.0 - ypos;
			xpos *= 2.0;
			ypos *= 2.0;
			xpos -= 1.0;
			ypos -= 1.0;

			mp = vec3(xpos, ypos, 0);
			mp /= camera_zoom;
			mp.y /= float(window_width)/float(window_height);
			mp -= -camera_pos
				* vec3(1, -float(window_width)/float(window_height), 1);
			mp.y /= float(bitmapA.HEIGHT)/float(bitmapA.WIDTH);

			mp.x += 1.0;
			mp.y += 1.0;
			mp.x /= 2.0;
			mp.y /= 2.0;
			mp.x *= float(bitmapA.WIDTH);
			mp.y *= float(bitmapA.HEIGHT);
			mp.x = max(mp.x, 0.0f);
			mp.y = max(mp.y, 0.0f);
			mp.x = min(mp.x, float(bitmapA.WIDTH-1));
			mp.y = min(mp.y, float(bitmapA.HEIGHT-1));

			mp.x -= float(bitmapA.WIDTH/3);
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Debug window
		{
			if (ImGui::Button("Save"))
				global_settings.save_settings_to_file();
			if (ImGui::Button("Load"))
				global_settings.load_settings_from_file();
			if (ImGui::Button("Reload"))
				reload_procedure();
			if (ImGui::Button("Soft reload"))
				soft_reload_procedure();

			constexpr std::size_t size_t_step = 1;
			ImGui::InputScalar("debug_vals[0]", ImGuiDataType_U64,
				&global_settings.debug_vals[0], &size_t_step);
			ImGui::InputScalar("debug_vals[1]", ImGuiDataType_U64,
				&global_settings.debug_vals[1], &size_t_step);
			ImGui::InputScalar("debug_vals[2]", ImGuiDataType_U64,
				&global_settings.debug_vals[2], &size_t_step);

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

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();

			ImGui::Text("mp: {%f, %f}", mp.x, mp.y);
			ImGui::Text("win: {%d, %d}",
				window_width, window_height);
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

		glClear(GL_COLOR_BUFFER_BIT);

		// Drawing the bitmap
		{
			// bitmapA.set(mp.y, mp.x + bitmapA.WIDTH/3, 0xff0000);
			// bitmapA.set(85, mp.x + bitmapA.WIDTH/3, 0xff0000);
			// bitmapA.set(85, 135 + bitmapA.WIDTH/3, 0xffffff);
			bitmapA.load_to_texture();
			bitmapA.draw(MVPb);
		}

		// Drawing the line (player)
		{
			mat4 MVP(1);
			MVP = scale(MVP, vec3(camera_zoom));
			MVP
			= scale(MVP, vec3(1, float(window_width)/float(window_height), 1));
			MVP = translate(MVP, -camera_pos *
					vec3(1, float(window_width)/float(window_height), 1)
				);
			auto [world_pos, gradient]
				= generator_C.get_tour_path_points(line_off);
			const float gradient_len
				= static_cast<float>(std::sqrt(len_sq(gradient)));
			const float gradient_sin
				= -static_cast<float>(gradient.x) / gradient_len;
			const float gradient_cos
				= static_cast<float>(gradient.y) / gradient_len;
			const mat4 rotate_mat {
				gradient_cos, -gradient_sin, 0, 0,
				gradient_sin, gradient_cos, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1,
			};
			world_pos.x /= generator_C.space_max.x;
			world_pos.y /= generator_C.space_max.y;
			world_pos.y = 1.0 - world_pos.y;
			world_pos *= 2.0;
			world_pos.x -= 1.0;
			world_pos.y -= 1.0;
			world_pos.y
				= world_pos.y * generator->ratio_hw;
			MVP = translate(MVP, vec3(static_cast<vec2>(world_pos), 0.0));
			MVP *= rotate_mat;
			MVP = scale(MVP, vec3(vec2(4, 1) / 400.0f, 1.0f));
			line.draw(MVP);
		}

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// double fps_cnt;
		{ // FPS cnter
			const double now = glfwGetTime();
			delta_time = now - timer_fps_cnter;
			// fps_cnt = 1.0 / delta_time;
			timer_fps_cnter = now;
		}
		{ // Debug output
			const auto now = std::chrono::high_resolution_clock::now();
			const auto delta_time = now - timer_logging;
			using namespace std::chrono_literals;
			if (delta_time >= 500ms) {
				timer_logging = now;

				// fprintf(stderr, "fps_cnt=%f\n", fps_cnt);
			}
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		std::this_thread::sleep_until(frame_end_time);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	global_settings.save_settings_to_file();

	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}
