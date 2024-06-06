// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "shader_A.hpp"

#include "shader_loader.hpp"
#include <settings.hpp>

void shader_A_fragment_common_uniforms_locations_t::init(GLuint program_id) {
	camera_pos_worldspace_uniform = glGetUniformLocation(program_id,
			"camera_pos_worldspace");
	light_pos_worldspace_uniform = glGetUniformLocation(program_id,
			"light_pos_worldspace");
	sun_direction_worldspace_uniform = glGetUniformLocation(program_id,
			"sun_direction_worldspace");
	light_color_uniform = glGetUniformLocation(program_id,
			"light_color");
	fog_color_uniform = glGetUniformLocation(program_id,
			"fog_color");
}

void shader_A_fragment_common_uniforms_locations_t::send_values(shader_A_fragment_common_uniforms_t values) {
	glUniform3f(camera_pos_worldspace_uniform,
		values.camera_pos_worldspace.x,
        values.camera_pos_worldspace.y,
        values.camera_pos_worldspace.z);
	glUniform3f(light_pos_worldspace_uniform,
		values.light_pos_worldspace.x,
        values.light_pos_worldspace.y,
        values.light_pos_worldspace.z);
	glUniform3f(sun_direction_worldspace_uniform,
		values.sun_direction_worldspace.x,
        values.sun_direction_worldspace.y,
        values.sun_direction_worldspace.z);
	glUniform3f(light_color_uniform,
		values.light_color.x,
        values.light_color.y,
        values.light_color.z);
	glUniform3f(fog_color_uniform,
		values.fog_color.x,
        values.fog_color.y,
        values.fog_color.z);
}

void shader_A_t::init() {
	GLuint vertex_shader_id = compile_shader(
			SHADER_A_VERTEX_PATH, GL_VERTEX_SHADER);
	GLuint fragment_shader_id = compile_shader(
			SHADER_A_FRAGMENT_PATH, GL_FRAGMENT_SHADER);

	program_id = link_program(2, vertex_shader_id, fragment_shader_id);

	delete_shader(vertex_shader_id);
	delete_shader(fragment_shader_id);

	// Get uniforms locations
	view_matrix_uniform = glGetUniformLocation(program_id, "V");
	model_matrix_uniform = glGetUniformLocation(program_id, "M");
	projection_matrix_uniform = glGetUniformLocation(program_id, "P");
	texture_sampler_uniform = glGetUniformLocation(program_id,
			"texture_sampler");

    common_fragment_uniforms_locations.init(program_id);
}

void shader_A_t::deinit() {
	delete_program(program_id);
}
