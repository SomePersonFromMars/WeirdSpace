// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "map_storage.hpp"

#include <cstdio>

#include <settings.hpp>
#include <useful.hpp>
#include <shader_loader.hpp>

void map_storage_t::load_settings() {
	desired_width
		= global_settings.map_unit_resolution
		* global_settings.map_width_in_units;
	if (global_settings.triple_map_size)
		desired_width *= 3;
	desired_height
		= global_settings.map_unit_resolution
		* global_settings.map_height_in_units;
}

void map_storage_t::reallocate_gpu_and_cpu_memory() {
	const int prev_width = width;
	const int prev_height = height;
	const int new_width = desired_width;
	const int new_height = desired_height;

	if (prev_width != new_width or
			prev_height != new_height) {
		if (content)
			delete[] content;
		if (new_width*new_height > 0)
			content = new uint8_t[new_width*new_height*4];
		else
			content = nullptr;

		glBindTexture(GL_TEXTURE_2D, get_texture_id());
		GL_GET_ERROR;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, new_width, new_height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		// glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
		GL_GET_ERROR;
	}

	width = desired_width;
	height = desired_height;
}

void map_storage_t::init_gl() {
	// Load shaders and generate shader program
	GLuint vertex_shader_id = compile_shader(
			SHADER_MAP_STORAGE_VERTEX_PATH, GL_VERTEX_SHADER);
	GLuint fragment_shader_id = compile_shader(
			SHADER_MAP_STORAGE_FRAGMENT_PATH, GL_FRAGMENT_SHADER);

	program_id = link_program(2, vertex_shader_id, fragment_shader_id);

	delete_shader(vertex_shader_id);
	delete_shader(fragment_shader_id);

	// Get uniform locations
	MVP_matrix_uniform = glGetUniformLocation(program_id, "MVP_matrix");
	texture_sampler_uniform = glGetUniformLocation(program_id,
			"texture_sampler");

	// Generate VAO and allocate vertex attributes' buffers
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &quad_positions_buffer_id);
	glGenBuffers(1, &quad_uvs_buffer_id);

	// Initialize VBOs with single instance data
	glBindBuffer(GL_ARRAY_BUFFER, quad_positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(quad_positions),
			quad_positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, quad_uvs_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(quad_uvs),
			quad_uvs, GL_STATIC_DRAW);

	// Add shader vertex attributes to the VAO
	// 1rst attribute buffer: vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_positions_buffer_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 2nd attribute buffer: UV coordinates
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, quad_uvs_buffer_id);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(0);

	// Generate texture
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, get_texture_id());

	// Set the texture wrapping/filtering options
	// (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			// GL_NEAREST_MIPMAP_NEAREST);
			// GL_NEAREST);
			GL_LINEAR);
			// GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void map_storage_t::deinit_gl() {
	glDeleteBuffers(1, &quad_positions_buffer_id);
	glDeleteBuffers(1, &quad_uvs_buffer_id);
	glDeleteVertexArrays(1, &vao_id);
	glDeleteTextures(1, &texture_id);
	delete_program(program_id);
}

map_storage_t::~map_storage_t() {
	if (content)
		delete[] content;
}

void map_storage_t::clear() {
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			set_rgb_value(y, x, 0);
			get_component_reference(y, x, 3) = 0;
		}
	}
}

void map_storage_t::load_from_cpu_to_gpu_memory() {
	glBindTexture(GL_TEXTURE_2D, get_texture_id());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, content);
	// glGenerateMipmap(GL_TEXTURE_2D);
	// WHERE; PRINT_U(glGetError());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void map_storage_t::load_from_gpu_to_cpu_memory() {
	glGetTextureImage(get_texture_id(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
			width*height*4, content);
}

void map_storage_t::draw(const glm::mat4 &MVP_matrix) {
	// Shader
	glUseProgram(program_id);

	// Uniforms and texture
	glUniformMatrix4fv(MVP_matrix_uniform,
		1, GL_FALSE, &MVP_matrix[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, get_texture_id());
	glUniform1i(texture_sampler_uniform,
			0);

	// Bind VAO and draw
	glBindVertexArray(vao_id);
	// glDrawArrays(GL_TRIANGLE_STRIP,
	glDrawArrays(GL_TRIANGLE_STRIP,
		0, 4);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}
