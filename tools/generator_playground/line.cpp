#include "line.hpp"

#include "settings.hpp"
#include "shader_loader.hpp"

line_t::line_t() {
	// Load shaders and generate shader program
	GLuint vertex_shader_id = compile_shader(
			SHADER_LINE_VERTEX_PATH, GL_VERTEX_SHADER);
	GLuint fragment_shader_id = compile_shader(
			SHADER_LINE_FRAGMENT_PATH, GL_FRAGMENT_SHADER);

	program_id = link_program(2, vertex_shader_id, fragment_shader_id);

	delete_shader(vertex_shader_id);
	delete_shader(fragment_shader_id);

	// Get uniform locations
	MVP_matrix_uniform = glGetUniformLocation(program_id, "MVP_matrix");

	// Generate VAO and allocate vertex attributes' buffers
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &quad_positions_buffer_id);

	// Initialize VBOs with single instance data
	glBindBuffer(GL_ARRAY_BUFFER, quad_positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(quad_positions),
			quad_positions, GL_STATIC_DRAW);

	// Add shader vertex attributes to the VAO
	// 1rst attribute buffer: vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_positions_buffer_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(0);
}

line_t::~line_t() {
	glDeleteBuffers(1, &quad_positions_buffer_id);
	glDeleteVertexArrays(1, &vao_id);
	delete_program(program_id);
}

void line_t::draw(const glm::mat4 &MVP_matrix) {
	// Shader
	glUseProgram(program_id);

	// Uniforms and texture
	glUniformMatrix4fv(MVP_matrix_uniform,
		1, GL_FALSE, &MVP_matrix[0][0]);

	// Bind VAO and draw
	glBindVertexArray(vao_id);
	// glDrawArrays(GL_TRIANGLE_STRIP,
	glDrawArrays(GL_TRIANGLE_STRIP,
		0, 4);
	glBindVertexArray(0);
}
