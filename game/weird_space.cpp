#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "callbacks.hpp"
#include "camera.hpp"
#include "shader_A.hpp"
#include "shader_world_A.hpp"
#include "shader_world_B.hpp"

#include "world_buffer.hpp"
#include "world_renderer.hpp"
#include "world_generator.hpp"
#include "player.hpp"

#include "settings.hpp"

int main( void )
{
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

#if WORLD_RENDER_METHOD == 1
	glEnable(GL_CULL_FACE);
#endif

	double delta_time = 0.0;

	shader_A_t shader_A;
	shader_world_t shader_world;
	shader_world_B_t shader_world_B;

	chunk_t::init_static(&shader_world);
	world_buffer_t world_buffer;
	world_renderer_t world_renderer(shader_world_B, world_buffer);
	world_renderer.init();

	std::chrono::time_point<std::chrono::high_resolution_clock>
		timer, now;
	long int elapsed;

	world_generator_t world_generator(world_buffer);
	for (int x = 0; x < 1; ++x) {
		for (int y = 0; y < 1; ++y) {
			world_generator.gen_chunk({x, y});

			timer = std::chrono::high_resolution_clock::now();
			world_renderer.preprocess_chunk({x, y});
			now = std::chrono::high_resolution_clock::now();
			elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
				now-timer).count();
			printf("%ldms\n", elapsed);

			chunk_t &sample_chunk = world_buffer.chunks[glm::ivec2(x, y)];
			sample_chunk.flush_content_for_drawing();
		}
	}
	timer = std::chrono::high_resolution_clock::now();
	world_renderer.finish_preprocessing();
	now = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
			now-timer).count();
	printf("%ldus\n", elapsed);

	player_t player(shader_A, world_buffer);
	player.debug_position = {chunk_t::WIDTH/2.0, chunk_t::HEIGHT, 0.5};
	player.set_position(player.debug_position);
	player.init();

	camera_t camera(glm::vec3(256, 120, -80)+glm::vec3(0, 1, -1.5),
			2*PI, 6.0f, 90.0f);

	callbacks_strct_t callbacks_strct(
			window,
			window_width,
			window_height,
			delta_time,
			camera,
			player
		);

	std::chrono::time_point<std::chrono::high_resolution_clock>
		timer_logging= std::chrono::high_resolution_clock::now();
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

#if WORLD_RENDER_METHOD == 1
		world_renderer.draw(light_pos,
			projection_matrix, view_matrix, model_matrix);
#elif WORLD_RENDER_METHOD == 2
		for (auto &p : world_buffer.chunks) {
			chunk_t &chunk = p.second;
			model_matrix[3][0] = p.first.x * chunk_t::WIDTH;
			model_matrix[3][2] = p.first.y * chunk_t::HEIGHT;
			chunk.draw(
				projection_matrix, view_matrix, model_matrix, light_pos);
		}
#endif

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

	world_renderer.deinit();
	chunk_t::deinit_static();
	glfwTerminate();
	return EXIT_SUCCESS;
}
