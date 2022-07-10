#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
using std::vector;

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;
GLint windowHeight = 768, windowWidth = 1536;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "utils/shader.hpp"
#include "utils/controls.hpp"

#include "shader_A.hpp"

#include "world_buffer.hpp"
#include "world_renderer.hpp"
#include "world_generator.hpp"
#include "player.hpp"

#include "settings.hpp"

void window_size_callback(GLFWwindow* window, int width, int height) {
	windowWidth = width;
	windowHeight = height;
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main( void )
{
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
	window = glfwCreateWindow( windowWidth, windowHeight, "Playground", NULL, NULL);
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

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

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

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	shader_A_t shader;
	shader.init();

	world_buffer_t world_buffer;
	world_renderer_t world_renderer(shader, world_buffer);
	world_renderer.init();
	world_generator_t world_generator(world_buffer);
	for (int x = 0; x < 10; ++x) {
		for (int y = 0; y < 10; ++y) {
			world_generator.gen_chunk({x, y});
			world_renderer.preprocess_chunk({x, y});
		}
	}
	// world_generator.gen_chunk({0, 0});
	// world_generator.gen_chunk({1, 0});
	// world_renderer.preprocess_chunk({0, 0});
	// world_renderer.preprocess_chunk({1, 0});
	world_renderer.finish_preprocessing();

	player_t player(shader);
	player.set_position({5, 10.5, 8});
	player.init();

	position = vec3(0, 16, 0);
	std::chrono::time_point<std::chrono::high_resolution_clock>
		timer_debugging= std::chrono::high_resolution_clock::now();
	double timer_fps_cnter = glfwGetTime();

	do {
		constexpr auto frame_min_duration
			= std::chrono::milliseconds(FRAME_MIN_DURATION);

		const auto frame_beg_time = std::chrono::high_resolution_clock::now();
		const auto frame_end_time
			= frame_beg_time + frame_min_duration;

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		computeMatricesFromInputs(windowWidth, windowHeight);
		glm::mat4 Projection = getProjectionMatrix();
		glm::mat4 View = getViewMatrix();
		glm::mat4 Model = glm::mat4(1.0f);

		world_renderer.draw(position, Projection, View, Model);
		player.draw(position, Projection, View, Model);

		double fps_cnt;
		{ // FPS cnter
			const double now = glfwGetTime();
			const double delta_time = now - timer_fps_cnter;
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
						position.x,
						position.y,
						position.z);
				fprintf(stderr, ", angle=(%f, %f)",
						horizontalAngle,
						verticalAngle);
				fprintf(stderr, ", fps_cnt=%f\n",
						fps_cnt);
			}
		}


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		std::this_thread::sleep_until(frame_end_time);
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
			glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO
	world_renderer.deinit();
	shader.deinit();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
