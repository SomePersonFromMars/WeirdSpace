#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
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
#include "utils/texture.hpp"

#include "chunk.hpp"
#include "world_buffer.hpp"
#include "world_renderer.hpp"
#include "world_generator.hpp"

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

	// Dark blue background
	glClearColor(0.0f, 0.8f, 1.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	world_buffer_t world_buffer;

	world_renderer_t world_renderer(world_buffer);
	world_renderer.init();

	world_generator_t world_generator(world_buffer);
	for (int x = 0; x < 20; ++x) {
		for (int y = 0; y < 20; ++y) {
			world_generator.gen_chunk({x, y});
			world_renderer.preprocess_chunk({x, y});
		}
	}
	// world_generator.gen_chunk(0);
	// world_generator.gen_chunk(1);
	// world_renderer.preprocess_chunk(0);
	// world_renderer.preprocess_chunk(1);
	world_renderer.finish_preprocessing();

	position = vec3(0, 16, 0);
	std::chrono::time_point<std::chrono::high_resolution_clock> timer = std::chrono::high_resolution_clock::now();

	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		computeMatricesFromInputs(windowWidth, windowHeight);
		glm::mat4 Projection = getProjectionMatrix();
		glm::mat4 View = getViewMatrix();
		glm::mat4 Model = glm::mat4(1.0f);

		world_renderer.draw(position, Projection, View, Model);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		auto now = std::chrono::high_resolution_clock::now();
		const auto delta_time = now - timer;

		using namespace std::chrono_literals;
		if (delta_time >= 500ms) {
			timer = now;

			fprintf(stderr, "pos=(%f, %f, %f)",
					position.x,
					position.y,
					position.z);
			fprintf(stderr, ", angle=(%f, %f)\n",
					horizontalAngle,
					verticalAngle);
		}

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
			glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO
	glDeleteVertexArrays(1, &VertexArrayID);
	world_renderer.deinit();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
