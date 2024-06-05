#pragma once
#ifndef SHADER_A_HPP
#define SHADER_A_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct shader_A_fragment_common_uniforms_t {
    glm::vec3 camera_pos_worldspace;
    glm::vec3 light_pos_worldspace;
    glm::vec3 sun_direction_worldspace;
    glm::vec3 light_color;
    glm::vec3 fog_color;
};

struct shader_A_fragment_common_uniforms_locations_t {
    GLuint camera_pos_worldspace_uniform;
	GLuint light_pos_worldspace_uniform;
	GLuint sun_direction_worldspace_uniform;
	GLuint light_color_uniform;
    GLuint fog_color_uniform;

    void init(GLuint program_id);
    void send_values(shader_A_fragment_common_uniforms_t values);
};

/*
	* Usage:

	* Construction
		shader_A_t shader; // Default constructor

	* Initialization:
		shader.init();

	* Drawing single VAO:
		glUseProgram(shader.program_id);
		glBindVertexArray(vao_id);
		glUniform*(shader.*_uniform, new_value);
		glDraw*(*);
		glBindVertexArray(0); // Unbinding not necessary

	* Deinitialization:
		shader.deinit();

	* Deconstruction
		// Default deconstructor
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
	GLuint texture_sampler_uniform;
    shader_A_fragment_common_uniforms_locations_t common_fragment_uniforms_locations;
};

#endif
