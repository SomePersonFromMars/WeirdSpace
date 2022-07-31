#include "shader_world_B.hpp"

#include "shader_loader.hpp"
#include "settings.hpp"

shader_world_B_t::shader_world_B_t() {
	GLuint vertex_shader_id = compile_shader(
			SHADER_WORLD_B_VERTEX_PATH, GL_VERTEX_SHADER);
	GLuint geometry_shader_id = compile_shader(
			SHADER_WORLD_B_GEOMETRY_PATH, GL_GEOMETRY_SHADER);
	GLuint fragment_shader_id = compile_shader(
			SHADER_A_FRAGMENT_PATH, GL_FRAGMENT_SHADER);

	program_id = link_program(3,
			vertex_shader_id, geometry_shader_id, fragment_shader_id);
	// program_id = link_program(2,
	// 		vertex_shader_id, fragment_shader_id);

	delete_shader(vertex_shader_id);
	delete_shader(geometry_shader_id);
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

shader_world_B_t::~shader_world_B_t() {
	delete_program(program_id);
}
