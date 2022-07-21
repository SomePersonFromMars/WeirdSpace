#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
using std::vector;

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "utils/shader.hpp"

#include "callbacks.hpp"
#include "camera.hpp"
#include "shader_A.hpp"

#include "world_buffer.hpp"
#include "world_renderer.hpp"
#include "world_generator.hpp"
#include "player.hpp"

#include "settings.hpp"

int main( void )
{
	GLFWwindow* window;
	GLint window_width = 1536, window_height = 768;

	// Initialise GLFW
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

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( window_width, window_height, "Playground", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Set the mouse at the center of the screen
	glfwPollEvents();

	// Background color
	const auto background_color = color_hex_to_vec3(SKY_COLOR);
	glClearColor(background_color.x, background_color.y, background_color.z,
			0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	double delta_time = 0.0;

	shader_A_t shader;
	shader.init();

	world_buffer_t world_buffer;
	world_renderer_t world_renderer(shader, world_buffer);
	world_renderer.init();

	world_buffer.get({0, 0, 0}) = block_type::sand;
	world_buffer.get({0, 1, 0}) = block_type::sand;
	world_buffer.get({1, 0, 0}) = block_type::sand;
	// world_buffer.get({2, 0, 0}) = block_type::sand;
	world_buffer.get({2, 1, 0}) = block_type::sand;
	world_buffer.get({3, 0, 0}) = block_type::sand;
	world_buffer.get({3, 1, 0}) = block_type::sand;
	world_buffer.get({4, 0, 0}) = block_type::sand;
	world_buffer.get({4, 1, 0}) = block_type::sand;
	world_buffer.get({4, 2, 0}) = block_type::sand;
	printf("block: %d\n",
			static_cast<int>(world_buffer.get({1, -1, 0})));
	printf("block: %d\n",
			static_cast<int>(world_buffer.get({-1, 0, 0})));
	printf("block: %d\n",
			static_cast<int>(world_buffer.get({1, 0, 0})));

	world_generator_t world_generator(world_buffer);
	for (int x = 0; x < 20; ++x) {
		for (int y = 0; y < 20; ++y) {
			world_generator.gen_chunk({x, y});
			world_renderer.preprocess_chunk({x, y});
		}
	}
	// world_generator.gen_chunk({0, 0});
	// world_generator.gen_chunk({1, 0});
	// world_renderer.preprocess_chunk({0, 0});
	// world_renderer.preprocess_chunk({1, 0});
	world_renderer.finish_preprocessing();

	player_t player(shader, world_buffer);
	// player.debug_position = {9+6, 9, 0.5};
	// player.debug_position = {0, 6, 0.5};
	player.debug_position = {0, 9, 0.5};
	// player.debug_position = {2, 2, 0.5};
	// player.debug_position = {2, -2, 0.5};
	// player.debug_position = {15.5002546, 9, 0.5};
	// player.debug_position = {2, 1, 0.5};
	// player.debug_position = {-1.5, 0, 0.5};
	// player.debug_position = {24, 5, 0.5};
	// player.debug_position = {5.703284, 0.296304, 0.5};
	player.set_position(player.debug_position);
	// player.set_position({2, 1, 0.5});
	// player.set_position({2.189584, 1.923374, 0.500000});
	// player.set_position({1.7, 1.5, 0.5});
	// player.set_position(glm::vec3(1.5, 1.0, 0.500000)
	// 		+ glm::vec3(0.2, 0.2, 0.0));
	// player.set_position({4.148619, 3.648619, 0.500000});
	player.init();

	// camera_t camera({9, 12, -3}, 2*PI, 6.0f, 120.0f);
	// camera_t camera(player.get_position()+glm::vec3(0, 1, -1.5),
	// 		2*PI, 6.0f, 120.0f);
	camera_t camera(glm::vec3(2, 1, 0.5)+glm::vec3(0, 1, -1.5),
			2*PI, 6.0f, 90.0f);

	// player.move_by(glm::vec2(3, 0));
	// player.on_axis_move_by(-0.5, &glm::vec3::x);
	// player.on_axis_move_by(0.9, &glm::vec3::y);
	// player.move_by(glm::vec2(3.0f, 0.0f));
	// player.move_by(glm::vec2(0.5f, 0.0f));
	// player.move_by(glm::vec2(-1.0f, -1.0f));
	// player.move_by(glm::vec2(-3.0f, -3.0f));
	// player.move_by(glm::vec2(-0.1f, 0.0f));
	// player.move_by(glm::vec2(-1, 0.0f));
	// player.move_by(glm::vec2(0.5, 0));
	// player.move_by(glm::vec2(10, 0));
	// player.move_by(glm::vec2(0, 10));
	// return EXIT_SUCCESS;

	callbacks_strct_t callbacks_strct(
			window,
			window_width,
			window_height,
			delta_time,
			camera,
			player
		);

	std::chrono::time_point<std::chrono::high_resolution_clock>
		timer_debugging= std::chrono::high_resolution_clock::now();
	double timer_fps_cnter = glfwGetTime();

	while (glfwWindowShouldClose(window) == GLFW_FALSE) {

		constexpr auto frame_min_duration
			= std::chrono::milliseconds(FRAME_MIN_DURATION);

		const auto frame_beg_time = std::chrono::high_resolution_clock::now();
		const auto frame_end_time
			= frame_beg_time + frame_min_duration;

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 Projection = camera.get_projection_matrix(
				window_width, window_height);
		glm::mat4 View = camera.get_view_matrix();
		glm::mat4 Model = glm::mat4(1.0f);

		const glm::vec3 light_pos
			= camera.get_position() + glm::vec3(0, 5, 0);
		world_renderer.draw(light_pos, Projection, View, Model);
		player.draw(light_pos, Projection, View);

		double fps_cnt;
		{ // FPS cnter
			const double now = glfwGetTime();
			delta_time = now - timer_fps_cnter;
			fps_cnt = 1.0 / delta_time;
			timer_fps_cnter = now;
		}
		{ // Debug output
			const auto now = std::chrono::high_resolution_clock::now();
			const auto delta_time = now - timer_debugging;
			using namespace std::chrono_literals;
			if (delta_time >= 500ms) {
				timer_debugging = now;

				fprintf(stderr, "pos=(%f, %f, %f)",
						player.get_position().x,
						player.get_position().y,
						player.get_position().z);
				fprintf(stderr, ", angle=(%f, %f)",
						camera.get_horizontal_angle(),
						camera.get_vertical_angle());
				fprintf(stderr, ", fps_cnt=%f\n",
						fps_cnt);
			}
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		callbacks_strct.handle_input();
		player.update_physics(delta_time);
		if (camera.get_following_mode())
			camera.follow(delta_time,
					player.get_position() + glm::vec3(0.5, 1, 0));
		std::this_thread::sleep_until(frame_end_time);
	}

	// Cleanup VBO
	world_renderer.deinit();
	shader.deinit();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return EXIT_SUCCESS;
}
