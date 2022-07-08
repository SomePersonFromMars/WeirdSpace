#include "shader_A.hpp"

void shader_A_t::init() {
	// Load and compile shaders
	program_id = LoadShaders(
		SHADER_A_VERTEX_PATH,
		SHADER_A_FRAGMENT_PATH
	);

	// Get uniform locations
	MVP_matrix_uniform = glGetUniformLocation(program_id, "MVP");
	view_matrix_uniform = glGetUniformLocation(program_id, "V");
	model_matrix_uniform = glGetUniformLocation(program_id, "M");
	projection_matrix_uniform = glGetUniformLocation(program_id, "P");
	light_uniform = glGetUniformLocation(program_id,
			"LightPosition_worldspace");
	light_color_uniform = glGetUniformLocation(program_id,
			"LightColor");
	texture_uniform = glGetUniformLocation(program_id,
			"myTextureSampler");
}

void shader_A_t::deinit() {
	glDeleteProgram(program_id);
}
