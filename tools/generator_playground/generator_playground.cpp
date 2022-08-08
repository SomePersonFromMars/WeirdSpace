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
#include "generator.hpp"

struct callbacks_strct_t {
	GLint window_width, window_height;

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

void fun_bitmap(bitmap_t &bitmap);

int32_t main(void) {
	GLFWwindow* window;

	callbacks_strct_t callbacksk_strct;
	GLint &window_width = callbacksk_strct.window_width;
	GLint &window_height = callbacksk_strct.window_height;

	window_width = 1080*2;
	window_height = 1080;

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

	// const auto background_color = color_hex_to_vec3(0xd6eefc);
	const auto background_color = color_hex_to_vec3(0);
	glClearColor(background_color.x, background_color.y, background_color.z,
			0.0f);

	bitmap_t bitmapA;
	generator_t generator;
	int resolution_div = 8;
	generator.generate_bitmap(bitmapA, resolution_div);
	bitmapA.load_to_texture();
	// fun_bitmap(bitmapA);
	// bitmapA.load_to_texture();

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

		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			generator.new_seed();
			generator.generate_bitmap(bitmapA, resolution_div);
			// generate_bitmap(bitmapA);
			bitmapA.load_to_texture();
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			camera_pos = vec3(0);
			camera_zoom = 1;
		}

		if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
			// resolution_div /= 2;
			resolution_div -= 1;
			if (resolution_div <= 0) resolution_div = 1;
			generator.generate_bitmap(bitmapA, resolution_div);
			bitmapA.load_to_texture();
		}
		if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
			resolution_div += 1;
			generator.generate_bitmap(bitmapA, resolution_div);
			bitmapA.load_to_texture();
		}

		const float move_off = delta_time * 1.0;
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

		const float zoom_off = delta_time * 1.0;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			camera_zoom *= 1+zoom_off;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			camera_zoom *= 1-zoom_off;
		}

		glClear(GL_COLOR_BUFFER_BIT);
		mat4 M(1);

		M = translate(M, -camera_pos *
				vec3(1, float(window_width)/float(window_height), 1)
			);
		M = scale(M, vec3(camera_zoom));
		bool enable_dualbitmap = false;
		M = scale(M, vec3(1, float(bitmapA.HEIGHT)/float(bitmapA.WIDTH), 1));
		M = scale(M, vec3(1, float(window_width)/float(window_height), 1));
		if (enable_dualbitmap) {
			M = scale(M, vec3(0.5, 0.5, 1));
		}

		if (!enable_dualbitmap) {
			bitmapA.draw(M);
		} else {
			M = translate(M, vec3(-1, 0, 0));
			bitmapA.draw(M);

			M = translate(M, vec3(2, 0, 0));
			bitmapA.draw(M);
		}

		double fps_cnt;
		{ // FPS cnter
			const double now = glfwGetTime();
			delta_time = now - timer_fps_cnter;
			fps_cnt = 1.0 / delta_time;
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

void fun_bitmap(bitmap_t &bitmap) {
	for (int x = 0; x < bitmap.WIDTH; ++x)
		for (int y = 0; y < bitmap.HEIGHT; ++y) {
			bitmap.get(y, x, 0) = 0;
			bitmap.get(y, x, 1) = 0;
			bitmap.get(y, x, 2) = 0;
		}

	auto seed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 1000;

	std::srand(seed);
	constexpr int PIX_DIM = 3;

	ivec2 pos(bitmap.WIDTH/2/PIX_DIM, bitmap.HEIGHT/2/PIX_DIM);
	for (int step = 0; step < 500; ++step) {

		for (int x = 0; x < PIX_DIM; ++x)
			for (int y = 0; y < PIX_DIM; ++y) {
				ivec2 tpos = pos;
				tpos.x *= PIX_DIM;
				tpos.y *= PIX_DIM;

				tpos.x += x;
				tpos.y += y;

				tpos.x = std::min(tpos.x, bitmap.WIDTH-1);
				tpos.y = std::min(tpos.y, bitmap.HEIGHT-1);

				uint32_t color = rand() & 0xff'ff'ff;
				// uint32_t color = 0xffffff;
				bitmap.get(tpos.y, tpos.x, 0) = (color & 0x0000ff) >> 0;
				bitmap.get(tpos.y, tpos.x, 1) = (color & 0x00ff00) >> 8;
				bitmap.get(tpos.y, tpos.x, 2) = (color & 0xff0000) >> 16;
			}

		pos.x += rand()%3-1;
		pos.y += rand()%3-1;
		pos.x = clamp(pos.x, 0, bitmap.WIDTH/PIX_DIM);
		pos.y = clamp(pos.y, 0, bitmap.HEIGHT/PIX_DIM);
	}

	for (int x = 0; x < bitmap.WIDTH; ++x) {
		bitmap.get(0, x, 2) = 0xff;
		bitmap.get(bitmap.HEIGHT-1, x, 2) = 0xff;
	}
	for (int y = 0; y < bitmap.HEIGHT; ++y) {
		bitmap.get(y, 0, 2) = 0xff;
		bitmap.get(y, bitmap.WIDTH-1, 2) = 0xff;
	}
}
