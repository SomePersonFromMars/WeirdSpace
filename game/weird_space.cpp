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

	// // Create and compile our GLSL program from the shaders
	// GLuint programID = LoadShaders( "vertex.vs", "fragment.fs" );

	// // Get a handle for our "MVP" uniform
	// GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	// GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	// GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	// GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// GLuint Texture = loadDDS("sand.dds");
	// GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	world_buffer_t world_buffer;

	world_renderer_t world_renderer(world_buffer);
	world_renderer.init();

	world_generator_t world_generator(world_buffer);
	world_generator.gen_chunk(0, 0);
	chunk_t &chunk = world_buffer.chunks[0];

	position = vec3(0, 16, 0);
	world_renderer.preprocess_chunk(0);
	world_renderer.finish_preprocessing();
	// chunk.update_buffers();

	// GLuint vertexbuffer;
	// glGenBuffers(1, &vertexbuffer);
	// glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// glBufferData(GL_ARRAY_BUFFER, chunk.vertex_buffer.size()*sizeof(GLfloat), &chunk.vertex_buffer[0], GL_STATIC_DRAW);

	// GLuint uvbuffer;
	// glGenBuffers(1, &uvbuffer);
	// glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	// glBufferData(GL_ARRAY_BUFFER, chunk.vertex_uvs.size()*sizeof(GLfloat), &chunk.vertex_uvs[0], GL_STATIC_DRAW);

	// GLuint normalbuffer;
	// glGenBuffers(1, &normalbuffer);
	// glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	// glBufferData(GL_ARRAY_BUFFER, chunk.vertex_normals.size()*sizeof(GLfloat), &chunk.vertex_normals[0], GL_STATIC_DRAW);

	std::chrono::time_point<std::chrono::high_resolution_clock> timer = std::chrono::high_resolution_clock::now();

	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// // Use our shader
		// glUseProgram(programID);

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		computeMatricesFromInputs(windowWidth, windowHeight);
		glm::mat4 Projection = getProjectionMatrix();
		glm::mat4 View = getViewMatrix();
		glm::mat4 Model = glm::mat4(1.0f);
		// glm::mat4 MVP = Projection * View * Model;

		world_renderer.draw(position, Projection, View, Model);

		// glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		// glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);
		// glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);

		// glm::vec3 lightPos = glm::vec3(10, 20, -10) + position;
		// glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// // Bind our texture in Texture Unit 0
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, Texture);
		// // Set our "myTextureSampler" sampler to use Texture Unit 0
		// glUniform1i(TextureID, 0);


		// // 1rst attribute buffer : vertices
		// glEnableVertexAttribArray(0);
		// glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		// glVertexAttribPointer(
		// 		0,                  // attribute
		// 		3,                  // size
		// 		GL_FLOAT,           // type
		// 		GL_FALSE,           // normalized?
		// 		0,                  // stride
		// 		(void*)0            // array buffer offset
		// 		);

		// // 2nd attribute buffer : UV coordinates
		// glEnableVertexAttribArray(1);
		// glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		// glVertexAttribPointer(
		// 		1,                  // attribute
		// 		2,                  // size
		// 		GL_FLOAT,           // type
		// 		GL_FALSE,           // normalized?
		// 		0,                  // stride
		// 		(void*)0            // array buffer offset
		// 		);

		// // 3rd attribute buffer : normals
		// glEnableVertexAttribArray(2);
		// glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		// glVertexAttribPointer(
		// 		2,                                // attribute
		// 		3,                                // size
		// 		GL_FLOAT,                         // type
		// 		GL_FALSE,                         // normalized?
		// 		0,                                // stride
		// 		(void*)0                          // array buffer offset
		// 		);

		// glDrawArrays(GL_TRIANGLES, 0, chunk.vertex_buffer.size()/3);

		// glDisableVertexAttribArray(0);
		// glDisableVertexAttribArray(1);
		// glDisableVertexAttribArray(2);

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
	// glDeleteBuffers(1, &vertexbuffer);
	// glDeleteBuffers(1, &uvbuffer);
	// glDeleteBuffers(1, &normalbuffer);
	// glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);
	// glDeleteProgram(programID);
	world_renderer.deinit();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
