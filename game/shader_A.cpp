#include "shader_A.hpp"

#include "shader_loader.hpp"
#include <settings.hpp>

void shader_A_t::init() {
	GLuint vertex_shader_id = compile_shader(
			SHADER_A_VERTEX_PATH, GL_VERTEX_SHADER);
	GLuint fragment_shader_id = compile_shader(
			SHADER_A_FRAGMENT_PATH, GL_FRAGMENT_SHADER);

	program_id = link_program(2, vertex_shader_id, fragment_shader_id);

	delete_shader(vertex_shader_id);
	delete_shader(fragment_shader_id);

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
	delete_program(program_id);
}
