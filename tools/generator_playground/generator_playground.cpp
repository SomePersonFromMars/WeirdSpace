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
		PRINT_ZU(++strct_ptr->generator->debug_val);
		strct_ptr->refresh_required = true;
	}
	if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
		if (strct_ptr->generator->debug_val != 0) {
			PRINT_ZU(--strct_ptr->generator->debug_val);
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
	generator->generate_bitmap(bitmapA, resolution_div);
	bitmapA.load_to_texture();

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
			generator->generate_bitmap(bitmapA, resolution_div);
			bitmapA.load_to_texture();
		}

		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			generator->new_seed();
			generator->generate_bitmap(bitmapA, resolution_div);
			bitmapA.load_to_texture();
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			camera_pos = vec3(0);
			camera_zoom = 1;
		}

		if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
			resolution_div -= 1;
			if (resolution_div <= 0) resolution_div = 1;
		}
		if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
			resolution_div += 1;
		}

		{
			const float move_off = delta_time * 1.0 / camera_zoom;
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
			const double move_off = delta_time * 2.0;
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
		// 	++generator->debug_val;
		// 	PRINT_ZU(generator->debug_val);
		// }
		// if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE) {
		// 	--generator->debug_val;
		// 	PRINT_ZU(generator->debug_val);
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

		glClear(GL_COLOR_BUFFER_BIT);

		// Drawing the bitmap
		{
			mat4 MVP(1);
			MVP = scale(MVP, vec3(camera_zoom));
			bool enable_dualbitmap = false;
			MVP = scale(MVP, vec3(1, float(window_width)/float(window_height), 1));
			if (enable_dualbitmap) {
				MVP = scale(MVP, vec3(0.5, 0.5, 1));
			}
			MVP = translate(MVP, -camera_pos *
					vec3(1, float(window_width)/float(window_height), 1)
				);
			MVP = scale(MVP,
					vec3(1, float(bitmapA.HEIGHT)/float(bitmapA.WIDTH), 1));
			if (!enable_dualbitmap) {
				bitmapA.draw(MVP);
			} else {
				MVP = translate(MVP, vec3(-1, 0, 0));
				bitmapA.draw(MVP);

				MVP = translate(MVP, vec3(2, 0, 0));
				bitmapA.draw(MVP);
			}
		}

		// Drawing the line (player)
		{
			mat4 MVP(1);
			MVP = scale(MVP, vec3(camera_zoom));
			MVP = scale(MVP, vec3(1, float(window_width)/float(window_height), 1));
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

	glfwTerminate();
	return EXIT_SUCCESS;
}
