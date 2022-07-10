#include "world_renderer.hpp"

world_renderer_t::world_renderer_t(
		shader_A_t &shader,
		world_buffer_t &buffer
	)
	:shader{shader}
	,buffer{buffer}
{  }

void world_renderer_t::init() {
	texture_id = load_texture("runtime/blocks_combined.png");

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &positions_buffer_id);
	glGenBuffers(1, &uvs_buffer_id);
	glGenBuffers(1, &normals_buffer_id);

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

	glBindVertexArray(0); // Unbind vao, not necessary
}

void world_renderer_t::clear_preprocessing_data() {
	positions_buffer.clear();
	uvs_buffer.clear();
	normals_buffer.clear();
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

				for (size_t i = 0; i < single_block_points_cnt; ++i) {
					const glm::vec3 pos(
						single_block_positions[3*i+0] + 2.0f * x,
						single_block_positions[3*i+1] + 2.0f * y,
						single_block_positions[3*i+2] + -2.0f * z
					);
					positions_buffer.push_back(pos.x / 2.0f + offset.x);
					positions_buffer.push_back(pos.y / 2.0f);
					positions_buffer.push_back(pos.z / 2.0f + offset.y);

					const glm::vec2 uv(
						single_block_uv[2*i+0],
						single_block_uv[2*i+1]
					);
					uvs_buffer.push_back(uv.x);
					uvs_buffer.push_back(uv.y);

					const glm::vec3 normal(
						single_block_normals[3*i+0],
						single_block_normals[3*i+1],
						single_block_normals[3*i+2]
					);
					normals_buffer.push_back(normal.x);
					normals_buffer.push_back(normal.y);
					normals_buffer.push_back(normal.z);
				}
			}
		}
	}
}

void world_renderer_t::finish_preprocessing() {
	glBindBuffer(GL_ARRAY_BUFFER, positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			positions_buffer.size()*sizeof(GLfloat),
			&positions_buffer[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, uvs_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			uvs_buffer.size()*sizeof(GLfloat),
			&uvs_buffer[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normals_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			normals_buffer.size()*sizeof(GLfloat),
			&normals_buffer[0], GL_STATIC_DRAW);
}

void world_renderer_t::draw(
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

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	// Set texture_sampler sampler to use Texture Unit 0
	glUniform1i(shader.texture_sampler_uniform,
			0);

	glBindVertexArray(vao_id);
	glDrawArrays(GL_TRIANGLES, 0, positions_buffer.size()/3);
	// glBindVertexArray(0); // Not necessary
}

void world_renderer_t::deinit() {
	glDeleteBuffers(1,  &positions_buffer_id);
	glDeleteBuffers(1,  &uvs_buffer_id);
	glDeleteBuffers(1,  &normals_buffer_id);
	glDeleteTextures(1, &texture_id);

	glDeleteVertexArrays(1, &vao_id);
}
