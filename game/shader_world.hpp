#pragma once
#ifndef SHADER_WORLD_B_HPP
#define SHADER_WORLD_B_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shader_A.hpp"

// Usage is the same as shader_A_t
struct shader_world_t {

	// * Loads and compiles the A shaders
	// * Gets the uniforms' locations
	void init();

	// * Deletes the program
	void deinit();

	// Program
	GLuint program_id;

	// Uniform buffer objects
	GLuint block_model_uniform_block_index;
	GLuint block_model_uniform_binding_point;

	// Uniforms
	// GLuint chunk_dim_uniform;
	GLuint view_matrix_uniform;
	GLuint model_matrix_uniform;
	GLuint projection_matrix_uniform;
	GLuint texture_sampler_uniform;

    shader_A_fragment_common_uniforms_locations_t common_fragment_uniforms_locations;
};

#endif
