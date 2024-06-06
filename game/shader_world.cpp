// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "shader_world.hpp"

#include "shader_loader.hpp"
#include <settings.hpp>

void shader_world_t::init() {
	GLuint vertex_shader_id = compile_shader(
			SHADER_WORLD_VERTEX_PATH, GL_VERTEX_SHADER);
	GLuint fragment_shader_id = compile_shader(
			SHADER_A_FRAGMENT_PATH, GL_FRAGMENT_SHADER);

	program_id = link_program(2,
			vertex_shader_id, fragment_shader_id);

	delete_shader(vertex_shader_id);
	delete_shader(fragment_shader_id);

	// Get uniform buffer objects' blocks indices
	block_model_uniform_block_index= glGetUniformBlockIndex(program_id,
			"block_model");
	block_model_uniform_binding_point = 0;
	glUniformBlockBinding(program_id,
		block_model_uniform_block_index, block_model_uniform_binding_point);

	// Get uniform locations
	view_matrix_uniform = glGetUniformLocation(program_id, "V");
	model_matrix_uniform = glGetUniformLocation(program_id, "M");
	projection_matrix_uniform = glGetUniformLocation(program_id, "P");

    common_fragment_uniforms_locations.init(program_id);
}

void shader_world_t::deinit() {
	delete_program(program_id);
}
