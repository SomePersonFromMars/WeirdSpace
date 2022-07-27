#pragma once
#ifndef SHADER_A_HPP
#define SHADER_A_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

/*
	* Usage:

	* Initialization:
		shader_A_t shader;

	* Drawing single VAO:
		glUseProgram(shader.program_id);
		glBindVertexArray(vao_id);
		glUniform*(shader.*_uniform, new_value);
		glDraw*(*);
		glBindVertexArray(0); // Unbinding not necessary

	* Deinitialization:
		// Destructor is called
*/

struct shader_A_t {

	// * Loads and compiles the A shaders
	// * Gets the uniforms' locations
	shader_A_t();

	// * Deletes the program
	~shader_A_t();

	// Program
	GLuint program_id;

	// Uniforms
	GLuint view_matrix_uniform;
	GLuint model_matrix_uniform;
	GLuint projection_matrix_uniform;
	GLuint light_pos_worldspace_uniform;
	GLuint light_color_uniform;
	GLuint texture_sampler_uniform;
};

#endif
