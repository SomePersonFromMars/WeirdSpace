#include "map_storage.hpp"

#include <cstdio>

#include <settings.hpp>
#include <useful.hpp>
#include <shader_loader.hpp>

void map_storage_t::load_settings() {
	width = global_settings.chunk_dim*6;
	if (global_settings.triple_map_size)
		width *= 3;
	height = global_settings.chunk_dim*3;
}

void map_storage_t::reallocate_gpu_and_cpu_memory() {
	static int prev_content_width = 0;
	static int prev_content_height = 0;
	static int prev_texture_width = 0;
	static int prev_texture_height = 0;

	const int new_content_width
		= global_settings.generate_with_gpu ? 0 : width;
	const int new_content_height
		= global_settings.generate_with_gpu ? 0 : height;
	const int new_texture_width
		= global_settings.generate_with_gpu ? width : 0;
	const int new_texture_height
		= global_settings.generate_with_gpu ? height : 0;

	if (
		prev_content_width != new_content_width or
		prev_content_height != new_content_height
		) {
		if (content)
			delete[] content;
		if (new_content_width*new_content_height > 0)
			content = new uint8_t[new_content_width*new_content_height*3];
		else
			content = nullptr;
	}

	if (
		prev_texture_width != new_texture_width or
		prev_texture_height != new_texture_height
		) {
		glBindTexture(GL_TEXTURE_2D, texture_id);
		GL_GET_ERROR;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
				GL_RGB, GL_UNSIGNED_BYTE, NULL);
		// glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
		GL_GET_ERROR;
	}

	prev_content_width = new_content_width;
	prev_content_height = new_content_height;
	prev_texture_width = new_texture_width;
	prev_texture_height = new_texture_height;
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
	glBindTexture(GL_TEXTURE_2D, texture_id);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
	// 		GL_RGB, GL_UNSIGNED_BYTE, NULL);
	// // glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

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

	// reallocate();
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

void map_storage_t::set(int y, int x, uint32_t color) {
	if (x < 0 || x >= width)
		return;
	if (y < 0 || y >= height)
		return;
	get(y, x, 2) = (color & 0x0000ff) >> 0;
	get(y, x, 1) = (color & 0x00ff00) >> 8;
	get(y, x, 0) = (color & 0xff0000) >> 16;
}

void map_storage_t::set(int y, int x, glm::u8vec3 color) {
	if (x < 0 || x >= width)
		return;
	if (y < 0 || y >= height)
		return;
	get(y, x, 2) = color.r;
	get(y, x, 1) = color.g;
	get(y, x, 0) = color.b;
}

void map_storage_t::clear() {
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			set(y, x, 0);
		}
	}
}

void map_storage_t::load_to_texture() {
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, content);
	// glGenerateMipmap(GL_TEXTURE_2D);
	// WHERE; PRINT_U(glGetError());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void map_storage_t::draw(const glm::mat4 &MVP_matrix) {
	// Shader
	glUseProgram(program_id);

	// Uniforms and texture
	glUniformMatrix4fv(MVP_matrix_uniform,
		1, GL_FALSE, &MVP_matrix[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
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
