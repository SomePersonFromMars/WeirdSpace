#include "shader_A.hpp"

void shader_A_t::init() {
	// Load and compile shaders
	program_id = LoadShaders(
		SHADER_A_VERTEX_PATH,
		SHADER_A_FRAGMENT_PATH
	);

	// Get uniform locations
	view_matrix_uniform = glGetUniformLocation(program_id, "V");
	model_matrix_uniform = glGetUniformLocation(program_id, "M");
	projection_matrix_uniform = glGetUniformLocation(program_id, "P");
	light_pos_worldspace_uniform = glGetUniformLocation(program_id,
			"light_pos_worldspace");
	light_color_uniform = glGetUniformLocation(program_id,
			"light_color");
	texture_sampler_uniform = glGetUniformLocation(program_id,
			"texture_sampler");
}

void shader_A_t::deinit() {
	glDeleteProgram(program_id);
}
