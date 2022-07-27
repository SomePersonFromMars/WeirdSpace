#include "chunk.hpp"

#include "texture_loader.hpp"
#include "useful.hpp"
#include "settings.hpp"

#include <cstdio>

shader_world_t *chunk_t::pshader;
GLuint chunk_t::texture_id;
GLuint chunk_t::block_model_uniform_buffer_id;

void chunk_t::init_static(shader_world_t *shader_ptr) {
	chunk_t::pshader = shader_ptr;

	// Load the combined texture
	texture_id = load_texture(TEXTURE_BLOCKS_COMBINED_PATH);

	// Generate, fill and bind the uniform buffer object
	glGenBuffers(1, &block_model_uniform_buffer_id);
	glBindBuffer(GL_UNIFORM_BUFFER, block_model_uniform_buffer_id);
	// Allocate the buffer
	glBufferData(GL_UNIFORM_BUFFER,
			sizeof(BLOCK_POSITIONS)+sizeof(BLOCK_UVS)+sizeof(BLOCK_NORMALS),
			nullptr, GL_STATIC_DRAW);

	// Fill the buffer
	glBufferSubData(GL_UNIFORM_BUFFER,
			0, sizeof(BLOCK_POSITIONS), &BLOCK_POSITIONS[0]);
	glBufferSubData(GL_UNIFORM_BUFFER,
			sizeof(BLOCK_POSITIONS), sizeof(BLOCK_UVS), &BLOCK_UVS[0]);
	glBufferSubData(GL_UNIFORM_BUFFER,
			sizeof(BLOCK_POSITIONS)+sizeof(BLOCK_UVS), sizeof(BLOCK_NORMALS),
			&BLOCK_NORMALS[0]);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Bind the whole buffer to GL_UNIFORM_BUFFER indexed buffer
	// at index shader.block_model_uniform_binding_point
	glBindBufferRange(GL_UNIFORM_BUFFER,
			pshader->block_model_uniform_binding_point,
			block_model_uniform_buffer_id, 0,
			sizeof(BLOCK_POSITIONS)+sizeof(BLOCK_UVS)+sizeof(BLOCK_NORMALS));
}

chunk_t::chunk_t()
{
	for (size_t x = 0; x < WIDTH; ++x)
		for (size_t y = 0; y < HEIGHT; ++y)
			for (size_t z = 0; z < DEPTH; ++z)
				content[x][y][z] = block_type::none;

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &content_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, content_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(content), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, content_buffer_id);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, 1, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	// glGenBuffers(1, &positions_buffer_id);
	// glGenBuffers(1, &uvs_buffer_id);
	// glGenBuffers(1, &normals_buffer_id);
	// glGenBuffers(1, &positions_instanced_buffer_id);

	// // Initialize VBOs with single instance data
	// glBindBuffer(GL_ARRAY_BUFFER, positions_buffer_id);
	// glBufferData(GL_ARRAY_BUFFER,
	// 		sizeof(single_block_positions),
	// 		single_block_positions, GL_STATIC_DRAW);

	// glBindBuffer(GL_ARRAY_BUFFER, uvs_buffer_id);
	// glBufferData(GL_ARRAY_BUFFER,
	// 		sizeof(single_block_uvs),
	// 		single_block_uvs, GL_STATIC_DRAW);

	// glBindBuffer(GL_ARRAY_BUFFER, normals_buffer_id);
	// glBufferData(GL_ARRAY_BUFFER,
	// 		sizeof(single_block_normals),
	// 		single_block_normals, GL_STATIC_DRAW);

	// // Add shader vertex attributes to the VAO
	// // 1rst attribute buffer: vertices
	// glEnableVertexAttribArray(0);
	// glBindBuffer(GL_ARRAY_BUFFER, positions_buffer_id);
	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// // 2nd attribute buffer: UV coordinates
	// glEnableVertexAttribArray(1);
	// glBindBuffer(GL_ARRAY_BUFFER, uvs_buffer_id);
	// glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// // 3rd attribute buffer: normals
	// glEnableVertexAttribArray(2);
	// glBindBuffer(GL_ARRAY_BUFFER, normals_buffer_id);
	// glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// // 4th attribute buffer: vertices
	// glEnableVertexAttribArray(3);
	// glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	// glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	// glVertexAttribDivisor(3, 1);
}

void chunk_t::deinit_static() {
	glDeleteTextures(1, &texture_id);

	glDeleteBuffers(1, &block_model_uniform_buffer_id);
}

chunk_t::~chunk_t() {
	// glDeleteBuffers(1,  &positions_buffer_id);
	// glDeleteBuffers(1,  &uvs_buffer_id);
	// glDeleteBuffers(1,  &normals_buffer_id);
	// glDeleteBuffers(1,  &positions_instanced_buffer_id);
	glDeleteBuffers(1, &content_buffer_id);
	glDeleteVertexArrays(1, &vao_id);
}

void chunk_t::flush_content_for_drawing() {
	glBindBuffer(GL_ARRAY_BUFFER, content_buffer_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(content), &content[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void chunk_t::draw(
	const glm::mat4 &projection_matrix,
	const glm::mat4 &view_matrix,
	const glm::mat4 &model_matrix,
	const glm::vec3 &light_pos
	) {
	glUseProgram(pshader->program_id);

	glUniform3i(pshader->chunk_dim_uniform,
		WIDTH,
		HEIGHT,
		DEPTH);

	glUniformMatrix4fv(pshader->model_matrix_uniform,
		1, GL_FALSE, &model_matrix[0][0]);
	glUniformMatrix4fv(pshader->view_matrix_uniform,
		1, GL_FALSE, &view_matrix[0][0]);
	glUniformMatrix4fv(pshader->projection_matrix_uniform,
		1, GL_FALSE, &projection_matrix[0][0]);

	const glm::vec3 light_color = color_hex_to_vec3(LIGHT_COLOR);
	glUniform3f(pshader->light_pos_worldspace_uniform,
		light_pos.x, light_pos.y, light_pos.z);
	glUniform3f(pshader->light_color_uniform,
		light_color.x, light_color.y, light_color.z);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glUniform1i(pshader->texture_sampler_uniform,
			0);

	glBindVertexArray(vao_id);
	glDrawArrays(GL_POINTS, 0, WIDTH*HEIGHT*DEPTH);
	glBindVertexArray(0);
}
