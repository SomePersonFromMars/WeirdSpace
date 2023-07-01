#include "app.hpp"

#include <cstdlib>
#include <thread>

#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "useful.hpp"

// App
app_t::app_t()
	:callbacks_strct(window_width, window_height)
	,generator_C(&bitmap_A)
{ }

// Init
void app_t::init() {
	global_settings.load_settings_from_file();
	init_opengl_etc();
	init_imgui();
	init_generator();
}

// Loop
void app_t::loop() {
	timer_logging = std::chrono::high_resolution_clock::now();
	timer_fps_cnter = glfwGetTime();
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

		loop_input();

		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, window_width, window_height);
		loop_generator();
		loop_imgui();

		double fps_cnt;
		{ // FPS cnter
			const double now = glfwGetTime();
			delta_time = now - timer_fps_cnter;
			fps_cnt = 1.0 / delta_time;
			timer_fps_cnter = now;
		}
		{ // Debug output
			const auto now = std::chrono::high_resolution_clock::now();
			const auto delta_time_n = now - timer_logging;
			using namespace std::chrono_literals;
			if (delta_time_n >= 500ms) {
				timer_logging = now;

				WHERE;
				fprintf(stderr, "fps_cnt=%f\n", fps_cnt);
				PRINT_U(glGetError());
			}
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		std::this_thread::sleep_until(frame_end_time);
	}
}

// Deinit
void app_t::deinit() {
	generator_C.deinit();
	deinit_imgui();

	global_settings.save_settings_to_file();

	glfwDestroyWindow(window);
	glfwTerminate();

	GL_GET_ERROR;
}


// Init subfunctions
static void framebuffer_size_callback(
		GLFWwindow* window, int width, int height);
void window_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window,
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
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);
}

// Loop subfunctions
void app_t::loop_input() {
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
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera_zoom *= 1-zoom_off;
	}

	// Calculate MVP for bitmap
	MVPb = mat4(1);
	{
		MVPb = scale(MVPb, vec3(camera_zoom));
		MVPb = scale(
			MVPb, vec3(1, float(window_width)/float(window_height), 1));

		MVPb = translate(MVPb, -camera_pos *
				vec3(1, float(window_width)/float(window_height), 1)
			);

		MVPb = scale(MVPb,
				vec3(1, float(bitmap_A.HEIGHT)/float(bitmap_A.WIDTH), 1));
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
		mp.y /= float(bitmap_A.HEIGHT)/float(bitmap_A.WIDTH);

		mp.x += 1.0;
		mp.y += 1.0;
		mp.x /= 2.0;
		mp.y /= 2.0;
		mp.x *= float(bitmap_A.WIDTH);
		mp.y *= float(bitmap_A.HEIGHT);
		mp.x = max(mp.x, 0.0f);
		mp.y = max(mp.y, 0.0f);
		mp.x = min(mp.x, float(bitmap_A.WIDTH-1));
		mp.y = min(mp.y, float(bitmap_A.HEIGHT-1));

		mp.x -= float(bitmap_A.WIDTH/3);
	}
}


// Callbacks
callbacks_strct_t::callbacks_strct_t(GLint &window_width, GLint &window_height)
	:window_width{window_width}
	,window_height{window_height}
{  }

void window_size_callback(GLFWwindow* window, int width, int height) {
	callbacks_strct_t * const strct_ptr
		= callbacks_strct_t::get_strct(window);

	strct_ptr->window_width = width;
	strct_ptr->window_height = height;
}

static void framebuffer_size_callback(
		GLFWwindow* window, int width, int height) {
	callbacks_strct_t * const strct_ptr
		= callbacks_strct_t::get_strct(window);
	strct_ptr->window_width = width;
	strct_ptr->window_height = height;

	// glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window,
		int key, [[maybe_unused]] int scancode,
		int action, [[maybe_unused]] int mods) {
	callbacks_strct_t * const strct_ptr
		= callbacks_strct_t::get_strct(window);

	if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {
		PRINT_ZU(++strct_ptr->generator_C->debug_vals[0]);
		strct_ptr->refresh_required = true;
	}
	if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
		if (strct_ptr->generator_C->debug_vals[0] != 0) {
			PRINT_ZU(--strct_ptr->generator_C->debug_vals[0]);
			strct_ptr->refresh_required = true;
		}
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		++strct_ptr->generator_C->debug_vals[1];
		strct_ptr->refresh_required = true;
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		if (strct_ptr->generator_C->debug_vals[1] != 0) {
			--strct_ptr->generator_C->debug_vals[1];
			strct_ptr->refresh_required = true;
		}
	}
}

