#include "app.hpp"

#include <chrono>
#include <cstdlib>
#include <thread>

#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <useful.hpp>

// App
app_t::app_t()
	:callbacks_strct(window_width, window_height)
	,map_generator(&map_storage)
{ }

// Init
void app_t::init() {
	global_settings.load_settings_from_file();
	init_opengl_etc();
	init_imgui();
	init_map_generator();
}

// Loop
void app_t::loop() {
	// timer_logging = std::chrono::high_resolution_clock::now();
    auto timer_fps_cnter = std::chrono::high_resolution_clock::now();
	delta_time = 0.0;

	while (glfwWindowShouldClose(window) == GLFW_FALSE) {
		constexpr auto frame_min_duration
			= std::chrono::milliseconds(FRAME_MIN_DURATION);

		const auto frame_beg_time = std::chrono::high_resolution_clock::now();
		const auto frame_end_time
			= frame_beg_time + frame_min_duration;

		if (callbacks_strct.refresh_required) {
			callbacks_strct.refresh_required = false;
			soft_reload_procedure();
		}

		in_loop_parse_input();

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, window_width, window_height);
		in_loop_draw_map();
		in_loop_update_imgui();

        const auto now = std::chrono::high_resolution_clock::now();
        delta_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - timer_fps_cnter).count()
            / 1000.0;
        timer_fps_cnter = now;

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		std::this_thread::sleep_until(frame_end_time);
	}
}

// Deinit
void app_t::deinit() {
	deinit_imgui();
	deinit_map_generator();
	deinit_opengl_etc();
	global_settings.save_settings_to_file();
}


// Init subfunctions
void framebuffer_size_callback(
		GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window,
		int key, int scancode, int action, int mods);

void app_t::init_opengl_etc() {
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		exit(-1);
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
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		exit(-1);
	}
	glGetError();

	// glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
	glfwSetWindowUserPointer(window,
			reinterpret_cast<void*>(&callbacks_strct));
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

#ifdef DEBUG
	PRINT_NL;
	PRINT_U(GL_NO_ERROR);
	PRINT_U(GL_INVALID_ENUM);
	PRINT_U(GL_INVALID_VALUE);
	PRINT_U(GL_INVALID_OPERATION);
	PRINT_U(GL_INVALID_FRAMEBUFFER_OPERATION);
	PRINT_U(GL_OUT_OF_MEMORY);
	PRINT_U(GL_STACK_UNDERFLOW);
	PRINT_U(GL_STACK_OVERFLOW);
	GLint max_texture_size;
	GLint max_compute_work_group_count[3];
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
	glGetIntegeri_v(
			GL_MAX_COMPUTE_WORK_GROUP_COUNT,
			0, &max_compute_work_group_count[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,
			1, &max_compute_work_group_count[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,
			2, &max_compute_work_group_count[2]);
	PRINT_D(max_texture_size);
	PRINT_D(max_compute_work_group_count[0]);
	PRINT_D(max_compute_work_group_count[1]);
	PRINT_D(max_compute_work_group_count[2]);
	PRINT_NL;
#endif
}

// Loop subfunctions
void app_t::in_loop_parse_input() {
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		reload_procedure();
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		camera_pos = vec3(0);
		camera_zoom = 1;
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
	// 	++map_generator->debug_vals[0];
	// 	PRINT_ZU(map_generator->debug_vals[0]);
	// }
	// if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE) {
	// 	--map_generator->debug_vals[0];
	// 	PRINT_ZU(map_generator->debug_vals[0]);
	// }

	const float zoom_off = delta_time * 1.0;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera_zoom *= 1+zoom_off;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera_zoom *= 1-zoom_off;
	}

	// Calculate MVP for map storage
	MVPb = mat4(1);
	{
		MVPb = scale(MVPb, vec3(camera_zoom));
		MVPb = scale(
			MVPb, vec3(1, float(window_width)/float(window_height), 1));

		MVPb = translate(MVPb, -camera_pos *
				vec3(1, float(window_width)/float(window_height), 1)
			);

		MVPb = scale(MVPb, vec3(
				1,
				float(map_storage.get_height())/float(map_storage.get_width()),
				1));
	}

	// Calculate pixel pointed by mouse cursor
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
		mp.y /= float(map_storage.get_height())/float(map_storage.get_width());

		mp.x += 1.0;
		mp.y += 1.0;
		mp.x /= 2.0;
		mp.y /= 2.0;
		mp.x *= float(map_storage.get_width());
		mp.y *= float(map_storage.get_height());
		mp.x = max(mp.x, 0.0f);
		mp.y = max(mp.y, 0.0f);
		mp.x = min(mp.x, float(map_storage.get_width()-1));
		mp.y = min(mp.y, float(map_storage.get_height()-1));

		mp.x -= float(map_storage.get_width()/3);
	}
}


// Deinit subfunctions
void app_t::deinit_opengl_etc() {
	glfwDestroyWindow(window);
	glfwTerminate();
	GL_GET_ERROR;
}


// Callbacks
callbacks_strct_t::callbacks_strct_t(GLint &window_width, GLint &window_height)
	:window_width{window_width}
	,window_height{window_height}
{  }

void framebuffer_size_callback(
		GLFWwindow* window, int width, int height) {
	callbacks_strct_t * const strct_ptr
		= callbacks_strct_t::get_strct(window);
	strct_ptr->window_width = width;
	strct_ptr->window_height = height;
	// glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window,
		int key, [[maybe_unused]] int scancode,
		int action, [[maybe_unused]] int mods) {
	callbacks_strct_t * const strct_ptr
		= callbacks_strct_t::get_strct(window);

	if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {
		PRINT_ZU(++strct_ptr->map_generator->debug_vals[0]);
		strct_ptr->refresh_required = true;
	}
	if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
		if (strct_ptr->map_generator->debug_vals[0] != 0) {
			PRINT_ZU(--strct_ptr->map_generator->debug_vals[0]);
			strct_ptr->refresh_required = true;
		}
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		++strct_ptr->map_generator->debug_vals[1];
		strct_ptr->refresh_required = true;
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		if (strct_ptr->map_generator->debug_vals[1] != 0) {
			--strct_ptr->map_generator->debug_vals[1];
			strct_ptr->refresh_required = true;
		}
	}
}
