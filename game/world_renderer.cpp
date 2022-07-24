#include "world_renderer.hpp"

#include <cstdio>

#include <glm/gtc/matrix_transform.hpp>
#include "texture_loader.hpp"

world_renderer_t::world_renderer_t(
		shader_A_t &shader,
		world_buffer_t &buffer
	)
	:shader{shader}
	,buffer{buffer}
{  }

void world_renderer_t::init() {
	// Generate OpenGL ids
	texture_id = load_texture("runtime/blocks_combined.png");

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &positions_buffer_id);
	glGenBuffers(1, &uvs_buffer_id);
	glGenBuffers(1, &normals_buffer_id);
	glGenBuffers(1, &positions_instanced_buffer_id);

	// Initialize VBOs with single instance data
	glBindBuffer(GL_ARRAY_BUFFER, positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(single_block_positions),
			single_block_positions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, uvs_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(single_block_uvs),
			single_block_uvs, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normals_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(single_block_normals),
			single_block_normals, GL_STATIC_DRAW);

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
}

void world_renderer_t::clear_preprocessing_data() {
	positions_instanced_buffer.clear();
}

void world_renderer_t::preprocess_chunk(const glm::ivec2 &chunk_pos) {
	const auto &content = buffer.chunks[chunk_pos].content;
	const glm::vec2 offset(
		chunk_pos.x * static_cast<float>(chunk_t::width),
		chunk_pos.y * static_cast<float>(chunk_t::depth)
	);

	for (size_t x = 0; x < content.size(); ++x) {
		for (size_t y = 0; y < content[x].size(); ++y) {
			for (size_t z = 0; z < content[x][y].size(); ++z) {
				if (content[x][y][z] == block_type::none) continue;

				const glm::vec3 block_pos(
					static_cast<float>(x) + offset.x,
					static_cast<float>(y),
					static_cast<float>(z) + offset.y
				);
				positions_instanced_buffer.push_back(block_pos.x);
				positions_instanced_buffer.push_back(block_pos.y);
				positions_instanced_buffer.push_back(block_pos.z);
			}
		}
	}
}

void world_renderer_t::finish_preprocessing() {
	glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			positions_instanced_buffer.size()*sizeof(GLfloat),
			&positions_instanced_buffer[0], GL_STATIC_DRAW);
}

void world_renderer_t::draw(
	const glm::vec3 &light_pos,
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

	const glm::vec3 light_color = color_hex_to_vec3(LIGHT_COLOR);

	glUniform3f(shader.light_pos_worldspace_uniform,
			light_pos.x, light_pos.y, light_pos.z);
	glUniform3f(shader.light_color_uniform,
			light_color.x,
			light_color.y,
			light_color.z);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glUniform1i(shader.texture_sampler_uniform,
			0);

	glBindVertexArray(vao_id);
	glDrawArraysInstanced(GL_TRIANGLES,
		0, single_block_points_cnt,
		positions_instanced_buffer.size()/3
	);
}

void world_renderer_t::deinit() {
	glDeleteBuffers(1,  &positions_buffer_id);
	glDeleteBuffers(1,  &uvs_buffer_id);
	glDeleteBuffers(1,  &normals_buffer_id);
	glDeleteBuffers(1,  &positions_instanced_buffer_id);
	glDeleteTextures(1, &texture_id);

	glDeleteVertexArrays(1, &vao_id);
}
