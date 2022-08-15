#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "callbacks.hpp"
#include "camera.hpp"
#include "shader_A.hpp"
#include "shader_world.hpp"

#include "world_buffer.hpp"
#include "world_generator.hpp"
#include "player.hpp"

#include "settings.hpp"

int32_t main(void) {
	GLFWwindow* window;
	GLint window_width = 1920, window_height = 1080;

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

	const auto background_color = color_hex_to_vec3(SKY_COLOR);
	glClearColor(background_color.x, background_color.y, background_color.z,
			0.0f);

	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	double delta_time = 0.0;

	shader_A_t shader_A;
	shader_world_t shader_world;

	chunk_t::init_static(&shader_world);
	world_buffer_t world_buffer;

	std::chrono::time_point<std::chrono::high_resolution_clock>
		timer, now;
	long int elapsed;

	const int CHUNKS_X_CNT = world_buffer.width;
	constexpr int CHUNKS_Z_CNT = 1;
	world_generator_t world_generator(world_buffer);
	for (int x = 0; x < CHUNKS_X_CNT; ++x) {
		for (int z = 0; z < CHUNKS_Z_CNT; ++z) {
			world_generator.gen_chunk({x, z});
		}
	}
	// // World end border
	// {
	// 	chunk_t &first_chunk = world_buffer.chunks[glm::ivec2(0, 0)];
	// 	chunk_t &last_chunk = world_buffer.chunks[glm::ivec2(
	// 			world_buffer.width-1, 0)];
	// 	for (int x = 0; x < 1; ++x)
	// 		for (int y = 60; y < chunk_t::HEIGHT; ++y)
	// 			for (int z = 3; z < chunk_t::DEPTH; ++z) {
	// 				int _x = x;
	// 				if (first_chunk.content[_x][y][z] == block_type::none)
	// 					first_chunk.content[_x][y][z] = block_type::brick;

	// 				_x = chunk_t::WIDTH-1-x;
	// 				if (last_chunk.content[_x][y][z] == block_type::none)
	// 					last_chunk.content[_x][y][z] = block_type::brick;
	// 			}
	// }
	// for (int z = 1; z <= 2; ++z)
	// 	for (int x = 142; x >= 140; --x)
	// 		for (int y = 19; y <= 30; ++y)
	// 			world_buffer.get(glm::ivec3(x, y, z)) = block_type::brick;
	// for (int x = 142; x >= 140; --x)
	// 	for (int y = 19; y <= 22; ++y)
	// 		world_buffer.get(glm::ivec3(x, y, 0)) = block_type::brick;

	for (int x = 0; x < CHUNKS_X_CNT; ++x) {
		for (int z = 0; z < CHUNKS_Z_CNT; ++z) {
			chunk_t &chunk = world_buffer.chunks[glm::ivec2(x, z)];
			if (x > 0)
				chunk.neighbors[0] = &world_buffer.chunks[glm::ivec2(x-1, z)];
			else if (x == 0)
				chunk.neighbors[0] = &world_buffer.chunks[glm::ivec2(
						world_buffer.width-1, z)];
			if (x < CHUNKS_X_CNT-1)
				chunk.neighbors[1] = &world_buffer.chunks[glm::ivec2(x+1, z)];
			else if (x == CHUNKS_X_CNT-1)
				chunk.neighbors[1] = &world_buffer.chunks[glm::ivec2(0, z)];

			if (z > 0)
				chunk.neighbors[4] = &world_buffer.chunks[glm::ivec2(x, z-1)];
			if (z < CHUNKS_Z_CNT-1)
				chunk.neighbors[5] = &world_buffer.chunks[glm::ivec2(x, z+1)];

			chunk.preprocess();
			chunk.send_preprocessed_to_gpu();
		}
	}

	player_t player(shader_A, world_buffer);
	// player.debug_position = {chunk_t::WIDTH/2.0, chunk_t::HEIGHT, 0.5};
	player.debug_position = {chunk_t::WIDTH/2.0, chunk_t::HEIGHT,
		float(chunk_t::DEPTH/2)+0.5};
	// player.debug_position = {0, chunk_t::HEIGHT, 0.5};
	player.set_position(player.debug_position);
	player.init();

	camera_t camera(glm::vec3(34.878933, 79.936882, -19.364670),
			// 2*PI, 6.0f,
			5.851774, 5.900709,
			90.0f);
	camera.switch_following_mode();

	callbacks_strct_t callbacks_strct(
			window,
			window_width,
			window_height,
			delta_time,
			camera,
			player
		);

	std::chrono::time_point<std::chrono::high_resolution_clock>
		timer_logging = std::chrono::high_resolution_clock::now();
	double timer_fps_cnter = glfwGetTime();

	while (glfwWindowShouldClose(window) == GLFW_FALSE) {

		constexpr auto frame_min_duration
			= std::chrono::milliseconds(FRAME_MIN_DURATION);

		const auto frame_beg_time = std::chrono::high_resolution_clock::now();
		const auto frame_end_time
			= frame_beg_time + frame_min_duration;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection_matrix = camera.get_projection_matrix(
				window_width, window_height);
		glm::mat4 view_matrix = camera.get_view_matrix();
		glm::mat4 model_matrix = glm::mat4(1);

		const glm::vec3 light_pos
			= camera.get_position() + glm::vec3(0, 5, 0);

		for (auto &p : world_buffer.chunks) {
			chunk_t &chunk = p.second;
			model_matrix[3][0] = p.first.x * chunk_t::WIDTH;
			model_matrix[3][2] = p.first.y * chunk_t::DEPTH;
			chunk.draw(
				projection_matrix, view_matrix, model_matrix, light_pos);

			if (p.first.x == 0) {
				model_matrix[3][0] = world_buffer.width * chunk_t::WIDTH;
				chunk.draw(
					projection_matrix, view_matrix, model_matrix, light_pos);
			}
			if (p.first.x == world_buffer.width-1) {
				model_matrix[3][0] = -1 * chunk_t::WIDTH;
				chunk.draw(
					projection_matrix, view_matrix, model_matrix, light_pos);
			}
		}

		player.draw(light_pos, projection_matrix, view_matrix);

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

				fprintf(stderr, "pos=(%f, %f, %f)",
						camera.get_position().x,
						camera.get_position().y,
						camera.get_position().z);
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

	chunk_t::deinit_static();
	glfwTerminate();
	return EXIT_SUCCESS;
}
