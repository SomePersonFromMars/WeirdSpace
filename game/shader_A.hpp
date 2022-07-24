#pragma once
#ifndef SHADER_A_HPP
#define SHADER_A_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

/*
	* Usage:

	* Initialization:
		shader_A_t shader;
		shader.init();

	* Drawing single VAO:
		glUseProgram(shader.program_id);
		glBindVertexArray(vao_id);
		glUniform*(shader.*_uniform, new_value);
		glDraw*(*);
		glBindVertexArray(0); // Unbinding not necessary

	* Deinitialization:
		shader.deinit();
*/

struct shader_A_t {

	// * Loads and compiles the A shaders
	// * Gets the uniforms' locations
	void init();

	// * Deletes the program
	void deinit();

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
