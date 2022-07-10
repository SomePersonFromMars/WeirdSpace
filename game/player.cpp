#include "player.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "utils/shader.hpp"
#include "utils/texture.hpp"

player_t::player_t(shader_A_t &shader)
	:shader{shader}
{  }

void player_t::init() {
	// Generate OpenGL ids
	texture_id = load_texture("runtime/player.png");

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &positions_buffer_id);
	glGenBuffers(1, &uvs_buffer_id);
	glGenBuffers(1, &normals_buffer_id);
	glGenBuffers(1, &positions_instanced_buffer_id);

	// Initialize VBOs with single instance data
	glBindBuffer(GL_ARRAY_BUFFER, positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertices_positions),
			vertices_positions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, uvs_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertices_uvs),
			vertices_uvs, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normals_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertices_normals),
			vertices_normals, GL_STATIC_DRAW);

	// Add shader vertex attributes to the VAO
	// 1rst attribute buffer: vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, positions_buffer_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 2nd attribute buffer: UV coordinates
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvs_buffer_id);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 3rd attribute buffer: normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normals_buffer_id);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 4th attribute buffer: vertices
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(3, 1);

	// Temporarily here
	glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(position),
			&position, GL_STATIC_DRAW);

	glBindVertexArray(0); // Unbind vao, not necessary
}

void player_t::draw(
	const glm::vec3 &camera_pos,
	const glm::mat4 &projection_matrix,
	const glm::mat4 &view_matrix,
	const glm::mat4 &model_matrix
	) {
	glUseProgram(shader.program_id);

	glUniformMatrix4fv(shader.model_matrix_uniform,
			1, GL_FALSE, &model_matrix[0][0]);
	glUniformMatrix4fv(shader.view_matrix_uniform,
			1, GL_FALSE, &view_matrix[0][0]);
	glUniformMatrix4fv(shader.projection_matrix_uniform,
			1, GL_FALSE, &projection_matrix[0][0]);

	const glm::vec3 light_pos = glm::vec3(0, 0, 0) + camera_pos;
	const glm::vec3 light_color = color_hex_to_vec3(LIGHT_COLOR);

	glUniform3f(shader.light_pos_worldspace_uniform,
			light_pos.x, light_pos.y, light_pos.z);
	glUniform3f(shader.light_color_uniform,
			light_color.x,
			light_color.y,
			light_color.z);

	// Bind texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	// Set texture_sampler sampler to use Texture Unit 0
	glUniform1i(shader.texture_sampler_uniform,
			0);

	glBindVertexArray(vao_id);
	glDrawArraysInstanced(GL_TRIANGLES,
		0, ARR_SIZE(vertices_positions)/3,
		1
	);

	// glBindVertexArray(0); // Not necessary
}

void player_t::deinit() {
	glDeleteBuffers(1,  &positions_buffer_id);
	glDeleteBuffers(1,  &uvs_buffer_id);
	glDeleteBuffers(1,  &normals_buffer_id);
	glDeleteBuffers(1,  &positions_instanced_buffer_id);
	glDeleteTextures(1, &texture_id);

	glDeleteVertexArrays(1, &vao_id);
}
