#include "bitmap.hpp"

#include "settings.hpp"
#include "useful.hpp"
#include "shader_loader.hpp"

bitmap_t::bitmap_t() {
	// Allocate content buffer
	content = new uint8_t[WIDTH*HEIGHT*3];

	// Load shaders and generate shader program
	GLuint vertex_shader_id = compile_shader(
			SHADER_VERTEX_PATH, GL_VERTEX_SHADER);
	GLuint fragment_shader_id = compile_shader(
			SHADER_FRAGMENT_PATH, GL_FRAGMENT_SHADER);

	program_id = link_program(2, vertex_shader_id, fragment_shader_id);

	delete_shader(vertex_shader_id);
	delete_shader(fragment_shader_id);

	// Get uniform locations
	model_matrix_uniform = glGetUniformLocation(program_id, "model_matrix");
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

	// Set the texture wrapping/filtering options
	// (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			// GL_NEAREST_MIPMAP_NEAREST);
			// GL_NEAREST);
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
}

bitmap_t::~bitmap_t() {
	glDeleteBuffers(1, &quad_positions_buffer_id);
	glDeleteBuffers(1, &quad_uvs_buffer_id);
	glDeleteVertexArrays(1, &vao_id);
	glDeleteTextures(1, &texture_id);
	delete_program(program_id);

	delete[] content;
}

void bitmap_t::set(int y, int x, uint32_t color) {
	get(y, x, 2) = (color & 0x0000ff) >> 0;
	get(y, x, 1) = (color & 0x00ff00) >> 8;
	get(y, x, 0) = (color & 0xff0000) >> 16;
}

void bitmap_t::set(int y, int x, glm::u8vec3 color) {
	get(y, x, 2) = color.r;
	get(y, x, 1) = color.g;
	get(y, x, 0) = color.b;
}

void bitmap_t::load_to_texture() {
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0,
			GL_RGB, GL_UNSIGNED_BYTE, content);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void bitmap_t::draw(const glm::mat4 &model_matrix) {
	// Shader
	glUseProgram(program_id);

	// Uniforms and texture
	glUniformMatrix4fv(model_matrix_uniform,
		1, GL_FALSE, &model_matrix[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glUniform1i(texture_sampler_uniform,
			0);

	// Bind VAO and draw
	glBindVertexArray(vao_id);
	// glDrawArrays(GL_TRIANGLE_STRIP,
	glDrawArrays(GL_TRIANGLE_STRIP,
		0, 4);
	glBindVertexArray(0);
}
