#pragma once
#ifndef SHADER_WORLD_HPP
#define SHADER_WORLD_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Usage is the same as shader_A_t
struct shader_world_t {

	// * Loads and compiles the A shaders
	// * Gets the uniforms' locations
	shader_world_t();

	// * Deletes the program
	~shader_world_t();

	// Program
	GLuint program_id;

	// Uniform buffer objects
	GLuint block_model_uniform_block_index;
	GLuint block_model_uniform_binding_point;

	// Uniforms
	GLuint chunk_dim_uniform;
	GLuint view_matrix_uniform;
	GLuint model_matrix_uniform;
	GLuint projection_matrix_uniform;
	GLuint light_pos_worldspace_uniform;
	GLuint light_color_uniform;
	GLuint texture_sampler_uniform;
};

#endif
